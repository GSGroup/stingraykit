#ifndef STINGRAYKIT_FUTURE_H
#define STINGRAYKIT_FUTURE_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/exception.h>
#include <stingraykit/exception_ptr.h>
#include <stingraykit/shared_ptr.h>
#include <stingraykit/FunctionToken.h>
#include <stingraykit/toolkit.h>
#include <stingraykit/unique_ptr.h>
#include <stingraykit/thread/ConditionVariable.h>
#include <stingraykit/thread/DummyCancellationToken.h>
#include <stingraykit/thread/ICancellationToken.h>
#include <stingraykit/thread/Thread.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_threads
	 * @{
	 */

	struct future_status
	{
		STINGRAYKIT_ENUM_VALUES(
			ready,
			timeout
		);
		STINGRAYKIT_DECLARE_ENUM_CLASS(future_status);
	};

	namespace Detail
	{
		template<typename T>
		struct future_value_holder
		{
			typedef const T& ConstructValueT;
		private:
			T _value;
		public:
			future_value_holder(ConstructValueT value) : _value(value) {}
			operator T() { return _value; }
		};

		template<typename T>
		struct future_value_holder<T&>
		{
			typedef T ValueT;
			typedef T& ConstructValueT;
		private:
			typedef T* StoredT;
			StoredT _value;
		public:
			future_value_holder(ValueT& value) : _value(&value) {}
			operator ValueT&() { return *_value; }
		};


		template<typename ResultType>
		struct future_callback
		{
			typedef function<void()> FunctionType;

		private:
			FunctionType			_function;
			FutureExecutionTester	_tester;

		public:
			future_callback(const FunctionType& function, const FutureExecutionTester& tester) : _function(function), _tester(_tester)
			{ }

			void invoke()
			{
				LocalExecutionGuard guard(_tester);

				if (guard)
					_function();
			}
		};


		template<typename T>
		class future_result
		{
		public:
			typedef future_value_holder<T> WrappedResultType;
			typedef optional<WrappedResultType> OptionalValue;

		private:
			OptionalValue	_value;
			exception_ptr	_exception;

		public:
			future_result() { }
			future_result(const T& value) : _value(value) { }
			future_result(const exception_ptr& exception) : _exception(exception) { }

			bool has_exception() const	{ return _exception; }
			bool has_value() const		{ return _value; }

			T get()
			{
				if (_exception)
					rethrow_exception(_exception);
				STINGRAYKIT_CHECK(_value, OperationCanceledException());
				return *_value;
			}
		};


		template<>
		class future_result<void>
		{
			bool			_value;
			exception_ptr	_exception;

		public:
			future_result() : _value(false) { }
			future_result(bool value) : _value(value) { }
			future_result(const exception_ptr& exception) : _value(false), _exception(exception) { }

			bool has_exception() const	{ return _exception; }
			bool has_value() const		{ return _value; }

			void get()
			{
				if (_exception)
					rethrow_exception(_exception);
				STINGRAYKIT_CHECK(_value, OperationCanceledException());
			}
		};


		template<typename T>
		class future_impl_base
		{
		public:
			typedef future_callback<T> Callback;
			typedef typename Callback::FunctionType CallbackFunction;

		protected:
			typedef future_result<T> ResultType;

			Mutex					_mutex;
			ConditionVariable		_condition;
			ResultType				_result;
			optional<Callback>		_callback;

		public:
			bool is_ready() const						{ MutexLock l(_mutex); return _result.has_value() || _result.has_exception(); }
			bool has_exception() const					{ MutexLock l(_mutex); return _result.has_exception(); }
			bool has_value() const						{ MutexLock l(_mutex); return _result.has_value(); }
			void wait(const ICancellationToken& token)	{ MutexLock l(_mutex); do_wait(token); }

			future_status wait_for(TimeDuration duration, const ICancellationToken& token)
			{ MutexLock l(_mutex); return do_timed_wait(duration, token); }

			future_status wait_until(const Time& absTime, const ICancellationToken& token)
			{ MutexLock l(_mutex); return do_timed_wait(absTime - Time::Now(), token); }

			T get()
			{
				MutexLock l(_mutex);
				do_wait(DummyCancellationToken());
				return _result.get();
			}

			Token set_callback(const CallbackFunction& callback)
			{
				MutexLock l(_mutex);

				TaskLifeToken lifeToken;

				_callback.reset();
				_callback = Callback(callback, lifeToken.GetExecutionTester());

				 return MakeToken<FunctionToken>(bind(&TaskLifeToken::Release, lifeToken));
			}

			void set_exception(exception_ptr ex)
			{
				MutexLock l(_mutex);
				if (is_ready())
					return;

				_result = ResultType(ex);
				notify_ready();
			}

		protected:
			void notify_ready()
			{
				_condition.Broadcast();
				if (_callback)
					_callback->invoke();
			}

			void do_wait(const ICancellationToken& token)
			{
				if(is_ready())
					return;

				while (!is_ready() && token)
					_condition.Wait(_mutex, token);
			}

			future_status do_timed_wait(TimeDuration duration, const ICancellationToken& token)
			{
				if (is_ready())
					return future_status::ready;

				while (!is_ready() && token)
					if (!_condition.TimedWait(_mutex, duration, token))
						break;

				return is_ready() ?	future_status::ready : future_status::timeout;
			}
		};


		template<typename T>
		class future_impl : public future_impl_base<T>
		{
			typedef future_impl_base<T> Base;
		public:
			void set_value(typename future_value_holder<T>::ConstructValueT value)
			{
				MutexLock l(this->_mutex);
				STINGRAYKIT_CHECK(!this->is_ready(), PromiseAlreadySatisfied());
				this->_result = typename Base::ResultType(value);
				this->notify_ready();
			}
		};


		template<>
		class future_impl<void> : public future_impl_base<void>
		{
			typedef future_impl_base<void> Base;
		public:
			void set_value()
			{
				MutexLock l(this->_mutex);
				STINGRAYKIT_CHECK(!this->is_ready(), PromiseAlreadySatisfied());
				this->_result = typename Base::ResultType(true);
				this->notify_ready();
			}
		};
	}

	template<typename ResultType>
	class future;

	template<typename ResultType>
	class shared_future;

	template<typename ResultType>
	class promise;

	template<typename ResultType>
	class shared_future
	{
	private:
		typedef shared_ptr<Detail::future_impl<ResultType> > ImplPtr;
		ImplPtr _impl;

	public:
		shared_future() {}
		~shared_future() {}
		shared_future(const shared_future &other) : _impl(other._impl) {}
		shared_future& operator=(const shared_future &other) { _impl = other._impl; return *this; }
		void swap(shared_future& other)	{ _impl.swap(other._impl); }

		bool valid() const				{ return _impl; }
		bool is_ready() const			{ check_valid(); return _impl->is_ready(); }
		bool has_exception() const		{ check_valid(); return _impl->has_exception(); }
		bool has_value() const			{ check_valid(); return _impl->has_value(); }

		ResultType get() const			{ check_valid(); return _impl->get(); }

		void wait(const ICancellationToken& token = DummyCancellationToken()) const										{ check_valid(); _impl->wait(token); }
		future_status wait_for(TimeDuration duration, const ICancellationToken& token = DummyCancellationToken()) const	{ check_valid(); return _impl->wait_for(duration, token); }
		future_status wait_until(const Time& absTime, const ICancellationToken& token = DummyCancellationToken()) const	{ check_valid(); return _impl->wait_until(absTime, token); }

	private:
		shared_future(const ImplPtr& impl) : _impl(impl) {}
		friend shared_future<ResultType> future<ResultType>::share();
		void check_valid() const { STINGRAYKIT_CHECK(valid(), std::runtime_error("No async result is assigned to the shared_future!")); }
	};

	template<typename ResultType>
	class future
	{
		STINGRAYKIT_NONASSIGNABLE(future);

	private:
		typedef Detail::future_impl<ResultType> ImplType;
		STINGRAYKIT_DECLARE_PTR(ImplType);
		shared_ptr<ImplType> _impl;

	public:
		~future()	{}

		bool valid() const				{ return _impl; }
		bool is_ready() const			{ check_valid(); return _impl->is_ready(); }
		bool has_exception() const		{ check_valid(); return _impl->has_exception(); }
		bool has_value() const			{ check_valid(); return _impl->has_value(); }

		shared_future<ResultType> share()
		{
			shared_ptr<ImplType> ptr(_impl);
			_impl.reset();
			return shared_future<ResultType>(ptr);
		}

		ResultType get()
		{
			check_valid();
			shared_ptr<ImplType> impl(_impl);
			_impl.reset();
			return impl->get();
		}

		void wait(const ICancellationToken& token = DummyCancellationToken()) const									{ check_valid(); _impl->wait(token); }
		future_status wait_for(TimeDuration duration, const ICancellationToken& token = DummyCancellationToken())	{ check_valid(); return _impl->wait_for(duration, token); }
		future_status wait_until(const Time& absTime, const ICancellationToken& token = DummyCancellationToken())	{ check_valid(); return _impl->wait_until(absTime, token); }

	private:
		future(const ImplTypePtr& impl) : _impl(impl) {}
		friend future<ResultType> promise<ResultType>::get_future();
		void check_valid() const { STINGRAYKIT_CHECK(valid(), std::runtime_error("No async result is assigned to the future!")); }
	};

	template<typename ResultType>
	class promise
	{
		STINGRAYKIT_NONCOPYABLE(promise);

	public:
		typedef typename Detail::future_value_holder<ResultType>::ConstructValueT SetType;

	private:
		typedef Detail::future_impl<ResultType>	FutureImplType;

		shared_ptr<FutureImplType>	_futureImpl;
		bool						_futureRetrieved;

	public:

		promise() : _futureImpl(new FutureImplType), _futureRetrieved(false)
		{}

		~promise()
		{ _futureImpl->set_exception(make_exception_ptr(BrokenPromise())); }

		void swap(promise& other)
		{ _futureImpl.swap(other._futureImpl); }

		future<ResultType> get_future()
		{ STINGRAYKIT_CHECK(!_futureRetrieved, FutureAlreadyRetrieved()); _futureRetrieved = true; return future<ResultType>(_futureImpl); }

		void set_value(SetType result)
		{ _futureImpl->set_value(result); }

		void set_exception(exception_ptr ex)
		{ _futureImpl->set_exception(ex); }
	};

	template<>
	class promise<void>
	{
		STINGRAYKIT_NONCOPYABLE(promise);

	private:
		typedef void							ResultType;
		typedef Detail::future_impl<ResultType>	FutureImplType;

		shared_ptr<FutureImplType>	_futureImpl;
		bool						_futureRetrieved;

	public:
		promise() : _futureImpl(new FutureImplType), _futureRetrieved(false)
		{}

		~promise()
		{ _futureImpl->set_exception(make_exception_ptr(BrokenPromise())); }

		void swap(promise& other)
		{ _futureImpl.swap(other._futureImpl); }

		future<ResultType> get_future()
		{ STINGRAYKIT_CHECK(!_futureRetrieved, FutureAlreadyRetrieved()); _futureRetrieved = true; return future<ResultType>(_futureImpl); }

		void set_value()
		{ _futureImpl->set_value(); }

		void set_exception(exception_ptr ex)
		{ _futureImpl->set_exception(ex); }

	};

	/** @} */

}

#endif
