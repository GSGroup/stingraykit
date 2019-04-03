#ifndef STINGRAYKIT_FUTURE_H
#define STINGRAYKIT_FUTURE_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/ExceptionPtr.h>
#include <stingraykit/thread/ConditionVariable.h>
#include <stingraykit/thread/DummyCancellationToken.h>

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
			timeout,
			cancelled
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


		template<typename T>
		class future_result
		{
		public:
			typedef future_value_holder<T> WrappedResultType;
			typedef optional<WrappedResultType> OptionalValue;

		private:
			OptionalValue	_value;
			ExceptionPtr	_exception;

		public:
			future_result() { }
			future_result(const T& value) : _value(value) { }
			future_result(const ExceptionPtr& exception) : _exception(exception) { }

			bool has_exception() const	{ return _exception; }
			bool has_value() const		{ return _value; }

			T get()
			{
				if (_exception)
					RethrowException(_exception);
				STINGRAYKIT_CHECK(_value, OperationCancelledException());
				return *_value;
			}
		};


		template<>
		class future_result<void>
		{
			bool			_value;
			ExceptionPtr	_exception;

		public:
			future_result() : _value(false) { }
			future_result(bool value) : _value(value) { }
			future_result(const ExceptionPtr& exception) : _value(false), _exception(exception) { }

			bool has_exception() const	{ return _exception; }
			bool has_value() const		{ return _value; }

			void get()
			{
				if (_exception)
					RethrowException(_exception);
				STINGRAYKIT_CHECK(_value, OperationCancelledException());
			}
		};


		template<typename T>
		class future_impl_base
		{
		protected:
			typedef future_result<T> ResultType;

			Mutex					_mutex;
			ConditionVariable		_condition;
			ResultType				_result;

		public:
			bool is_ready() const						{ MutexLock l(_mutex); return _result.has_value() || _result.has_exception(); }
			bool has_exception() const					{ MutexLock l(_mutex); return _result.has_exception(); }
			bool has_value() const						{ MutexLock l(_mutex); return _result.has_value(); }

			future_status wait(const ICancellationToken& token)
			{ MutexLock l(_mutex); return do_wait(token); }

			T get()
			{
				MutexLock l(_mutex);
				do_wait(DummyCancellationToken());
				return _result.get();
			}

			void set_exception(ExceptionPtr ex)
			{
				MutexLock l(_mutex);
				if (is_ready())
					return;

				_result = ResultType(ex);
				notify_ready();
			}

		protected:
			void notify_ready()
			{ _condition.Broadcast(); }

			future_status do_wait(const ICancellationToken& token)
			{
				while (!_result.has_value() && !_result.has_exception())
					switch (_condition.Wait(_mutex, token))
					{
					case ConditionWaitResult::Broadcasted:	continue;
					case ConditionWaitResult::Cancelled:	return future_status::cancelled;
					case ConditionWaitResult::TimedOut:		return future_status::timeout;
					}

				return future_status::ready;
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
				this->_result = Base::ResultType(true);
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

		future_status wait(const ICancellationToken& token = DummyCancellationToken()) const
		{ check_valid(); return _impl->wait(token); }

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

		future_status wait(const ICancellationToken& token = DummyCancellationToken()) const
		{ check_valid(); return _impl->wait(token); }

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

		promise() : _futureImpl(make_shared<FutureImplType>()), _futureRetrieved(false)
		{}

		~promise()
		{ _futureImpl->set_exception(MakeExceptionPtr(BrokenPromise())); }

		void swap(promise& other)
		{ _futureImpl.swap(other._futureImpl); }

		future<ResultType> get_future()
		{ STINGRAYKIT_CHECK(!_futureRetrieved, FutureAlreadyRetrieved()); _futureRetrieved = true; return future<ResultType>(_futureImpl); }

		void set_value(SetType result)
		{ _futureImpl->set_value(result); }

		void set_exception(ExceptionPtr ex)
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
		promise() : _futureImpl(make_shared<FutureImplType>()), _futureRetrieved(false)
		{}

		~promise()
		{ _futureImpl->set_exception(MakeExceptionPtr(BrokenPromise())); }

		void swap(promise& other)
		{ _futureImpl.swap(other._futureImpl); }

		future<ResultType> get_future()
		{ STINGRAYKIT_CHECK(!_futureRetrieved, FutureAlreadyRetrieved()); _futureRetrieved = true; return future<ResultType>(_futureImpl); }

		void set_value()
		{ _futureImpl->set_value(); }

		void set_exception(ExceptionPtr ex)
		{ _futureImpl->set_exception(ex); }

	};

	/** @} */

}

#endif
