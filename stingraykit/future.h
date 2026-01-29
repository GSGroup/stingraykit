#ifndef STINGRAYKIT_FUTURE_H
#define STINGRAYKIT_FUTURE_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
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

		template < typename T >
		class future_result
		{
		public:
			using ConstructValueT = const T&;

		private:
			optional<T>		_value;

		public:
			future_result() { }
			future_result(const T& value) : _value(value) { }

			explicit operator bool () const { return _value.is_initialized(); }

			T get()
			{
				STINGRAYKIT_CHECK(_value, OperationCancelledException());
				return *_value;
			}
		};


		template < typename T >
		class future_result<T&>
		{
		public:
			using ConstructValueT = T&;

		private:
			T*				_value;

		public:
			future_result() : _value() { }
			future_result(T& value) : _value(&value) { }

			explicit operator bool () const { return _value; }

			T& get()
			{
				STINGRAYKIT_CHECK(_value, OperationCancelledException());
				return *_value;
			}
		};


		template < >
		class future_result<void>
		{
		private:
			bool			_value;

		public:
			future_result() : _value(false) { }
			future_result(bool value) : _value(value) { }

			explicit operator bool () const { return _value; }

			void get()
			{ STINGRAYKIT_CHECK(_value, OperationCancelledException()); }
		};


		template < typename T >
		class future_impl
		{
			using ResultType = future_result<T>;

		private:
			Mutex					_mutex;
			ConditionVariable		_condition;

			ResultType				_result;
			ExceptionPtr			_exception;

		public:
			bool is_ready() const						{ MutexLock l(_mutex); return _result || _exception; }
			bool has_exception() const					{ MutexLock l(_mutex); return _exception.is_initialized(); }
			bool has_value() const						{ MutexLock l(_mutex); return static_cast<bool>(_result); }

			future_status wait(const ICancellationToken& token)
			{ MutexLock l(_mutex); return do_wait(token); }

			T get()
			{
				MutexLock l(_mutex);
				do_wait(DummyCancellationToken());

				if (_exception)
					STINGRAYKIT_RETHROW_EXCEPTION(_exception);

				return _result.get();
			}

			template < typename U = bool >
			void set_value(U&& value = true)
			{
				MutexLock l(_mutex);
				STINGRAYKIT_CHECK(!is_ready(), PromiseAlreadySatisfied());
				_result = ResultType(std::forward<U>(value));
				notify_ready();
			}

			void set_exception(ExceptionPtr ex)
			{
				MutexLock l(_mutex);
				if (is_ready())
					return;

				_exception = ex;
				notify_ready();
			}

		private:
			void notify_ready()
			{ _condition.Broadcast(); }

			future_status do_wait(const ICancellationToken& token)
			{
				while (!_result && !_exception)
					switch (_condition.Wait(_mutex, token))
					{
					case ConditionWaitResult::Broadcasted:	continue;
					case ConditionWaitResult::Cancelled:	return future_status::cancelled;
					case ConditionWaitResult::TimedOut:		return future_status::timeout;
					}

				return future_status::ready;
			}
		};

	}


	template < typename ResultType >
	class future;


	template < typename ResultType >
	class shared_future;


	template < typename ResultType >
	class promise;


	template < typename ResultType >
	class shared_future
	{
		using Impl = Detail::future_impl<ResultType>;
		STINGRAYKIT_DECLARE_PTR(Impl);

	private:
		ImplPtr			_impl;

	public:
		shared_future() { }
		~shared_future() { }
		shared_future(const shared_future& other) : _impl(other._impl) { }
		shared_future& operator= (const shared_future& other) { _impl = other._impl; return *this; }
		void swap(shared_future& other)	{ _impl.swap(other._impl); }

		bool valid() const				{ return _impl.is_initialized(); }
		bool is_ready() const			{ check_valid(); return _impl->is_ready(); }
		bool has_exception() const		{ check_valid(); return _impl->has_exception(); }
		bool has_value() const			{ check_valid(); return _impl->has_value(); }

		ResultType get() const			{ check_valid(); return _impl->get(); }

		future_status wait(const ICancellationToken& token = DummyCancellationToken()) const
		{ check_valid(); return _impl->wait(token); }

	private:
		explicit shared_future(const ImplPtr& impl) : _impl(impl) { }
		friend shared_future<ResultType> future<ResultType>::share();
		void check_valid() const { STINGRAYKIT_CHECK(valid(), InvalidFuturePromiseState()); }
	};


	template < typename ResultType >
	class future
	{
		STINGRAYKIT_NONASSIGNABLE(future);

	private:
		using Impl = Detail::future_impl<ResultType>;
		STINGRAYKIT_DECLARE_PTR(Impl);

	private:
		ImplPtr			_impl;

	public:
		future() { }
		~future() { }

		bool valid() const				{ return _impl.is_initialized(); }
		bool is_ready() const			{ check_valid(); return _impl->is_ready(); }
		bool has_exception() const		{ check_valid(); return _impl->has_exception(); }
		bool has_value() const			{ check_valid(); return _impl->has_value(); }

		shared_future<ResultType> share()
		{
			ImplPtr ptr(_impl);
			_impl.reset();
			return shared_future<ResultType>(ptr);
		}

		ResultType get()
		{
			check_valid();
			ImplPtr tmp;
			tmp.swap(_impl);
			return tmp->get();
		}

		future_status wait(const ICancellationToken& token = DummyCancellationToken()) const
		{ check_valid(); return _impl->wait(token); }

	private:
		explicit future(const ImplPtr& impl) : _impl(impl) { }
		friend future<ResultType> promise<ResultType>::get_future();
		void check_valid() const { STINGRAYKIT_CHECK(valid(), InvalidFuturePromiseState()); }
	};


	template < typename ResultType >
	class promise
	{
		STINGRAYKIT_NONCOPYABLE(promise);

	public:
		using SetType = typename Detail::future_result<ResultType>::ConstructValueT;

	private:
		using FutureImplType = Detail::future_impl<ResultType>;

	private:
		shared_ptr<FutureImplType>	_futureImpl;
		bool						_futureRetrieved;

	public:
		promise() : _futureImpl(make_shared_ptr<FutureImplType>()), _futureRetrieved(false)
		{ }

		~promise()
		{ _futureImpl->set_exception(MakeExceptionPtr(BrokenPromise())); }

		void swap(promise& other)
		{
			_futureImpl.swap(other._futureImpl);
			std::swap(_futureRetrieved, other._futureRetrieved);
		}

		future<ResultType> get_future()
		{ STINGRAYKIT_CHECK(!_futureRetrieved, FutureAlreadyRetrieved()); _futureRetrieved = true; return future<ResultType>(_futureImpl); }

		void set_value(SetType result)
		{ _futureImpl->set_value(result); }

		void set_exception(ExceptionPtr ex)
		{ _futureImpl->set_exception(ex); }
	};


	template < >
	class promise<void>
	{
		STINGRAYKIT_NONCOPYABLE(promise);

	private:
		using ResultType = void;
		using FutureImplType = Detail::future_impl<ResultType>;

	private:
		shared_ptr<FutureImplType>	_futureImpl;
		bool						_futureRetrieved;

	public:
		promise() : _futureImpl(make_shared_ptr<FutureImplType>()), _futureRetrieved(false)
		{ }

		~promise()
		{ _futureImpl->set_exception(MakeExceptionPtr(BrokenPromise())); }

		void swap(promise& other)
		{
			_futureImpl.swap(other._futureImpl);
			std::swap(_futureRetrieved, other._futureRetrieved);
		}

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
