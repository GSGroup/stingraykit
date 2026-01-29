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
		class shared_state_result
		{
		private:
			optional<T>		_value;

		public:
			shared_state_result() { }

			explicit operator bool () const { return _value.is_initialized(); }

			void set(const T& value)
			{ _value.emplace(value); }

			void set(T&& value)
			{ _value.emplace(std::move(value)); }

			T get()
			{ return *STINGRAYKIT_REQUIRE_INITIALIZED(_value); }
		};


		template < typename T >
		class shared_state_result<T&>
		{
		private:
			T*				_value;

		public:
			shared_state_result() : _value() { }

			explicit operator bool () const { return _value; }

			void set(T& value)
			{ _value = &value; }

			T& get()
			{ return *STINGRAYKIT_REQUIRE_NOT_NULL(_value); }
		};


		template < >
		class shared_state_result<void>
		{
		private:
			bool			_value;

		public:
			shared_state_result() : _value(false) { }

			explicit operator bool () const { return _value; }

			void set()
			{ _value = true; }

			void get()
			{ }
		};


		template < typename T >
		class shared_state
		{
			using ResultType = shared_state_result<T>;

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
			{
				MutexLock l(_mutex);

				while (!_result && !_exception)
					switch (_condition.Wait(_mutex, token))
					{
					case ConditionWaitResult::Broadcasted:	continue;
					case ConditionWaitResult::Cancelled:	return future_status::cancelled;
					case ConditionWaitResult::TimedOut:		return future_status::timeout;
					}

				return future_status::ready;
			}

			T get()
			{
				MutexLock l(_mutex);

				while (true)
				{
					if (_exception)
						STINGRAYKIT_RETHROW_EXCEPTION(_exception);

					if (_result)
						return _result.get();

					_condition.Wait(_mutex);
				}
			}

			template < typename... Us >
			void set_value(Us&&... args)
			{
				MutexLock l(_mutex);
				STINGRAYKIT_CHECK(!is_ready(), PromiseAlreadySatisfied());
				_result.set(std::forward<Us>(args)...);
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
		};

	}


	template < typename ResultType >
	class future;


	template < typename ResultType >
	class shared_future;


	namespace Detail
	{

		template < typename ResultType >
		class promise_base
		{
			STINGRAYKIT_NONCOPYABLE(promise_base);

		protected:
			using SharedStateType = Detail::shared_state<ResultType>;

		protected:
			shared_ptr<SharedStateType>	_state;

		private:
			bool						_futureRetrieved;

		public:
			promise_base() : _state(make_shared_ptr<SharedStateType>()), _futureRetrieved(false)
			{ }

			promise_base(promise_base&& other) : _state(std::move(other._state)), _futureRetrieved(other._futureRetrieved)
			{ other._futureRetrieved = false; }

			~promise_base()
			{
				try
				{
					if (_state)
						_state->set_exception(MakeExceptionPtr(BrokenPromise()));
				}
				catch (...)
				{ }
			}

			promise_base& operator = (promise_base&& other)
			{
				promise_base tmp(std::move(other));
				swap(tmp);
				return *this;
			}

			void swap(promise_base& other)
			{
				_state.swap(other._state);
				std::swap(_futureRetrieved, other._futureRetrieved);
			}

			future<ResultType> get_future()
			{
				check_valid();
				STINGRAYKIT_CHECK(!_futureRetrieved, FutureAlreadyRetrieved());
				_futureRetrieved = true;
				return future<ResultType>(_state);
			}

			void set_exception(ExceptionPtr ex)
			{ check_valid(); _state->set_exception(ex); }

		protected:
			void check_valid() const { STINGRAYKIT_CHECK(_state, InvalidFuturePromiseState()); }
		};

	}


	template < typename ResultType >
	class promise : public Detail::promise_base<ResultType>
	{
		STINGRAYKIT_NONCOPYABLE(promise);
		STINGRAYKIT_DEFAULTMOVABLE(promise);

	private:
		using Base = Detail::promise_base<ResultType>;

	public:
		promise() { }

		void set_value(const ResultType& result)
		{ Base::check_valid(); Base::_state->set_value(result); }

		void set_value(ResultType&& result)
		{ Base::check_valid(); Base::_state->set_value(std::move(result)); }
	};


	template < typename ResultType >
	class promise<ResultType&> : public Detail::promise_base<ResultType&>
	{
		STINGRAYKIT_NONCOPYABLE(promise);
		STINGRAYKIT_DEFAULTMOVABLE(promise);

	private:
		using Base = Detail::promise_base<ResultType&>;

	public:
		promise() { }

		void set_value(ResultType& result)
		{ Base::check_valid(); Base::_state->set_value(result); }
	};


	template < >
	class promise<void> : public Detail::promise_base<void>
	{
		STINGRAYKIT_NONCOPYABLE(promise);
		STINGRAYKIT_DEFAULTMOVABLE(promise);

	private:
		using Base = Detail::promise_base<void>;

	public:
		promise() { }

		void set_value()
		{ Base::check_valid(); Base::_state->set_value(); }
	};


	namespace Detail
	{

		template < typename ResultType >
		class future_base
		{
		protected:
			using SharedStateType = Detail::shared_state<ResultType>;
			STINGRAYKIT_DECLARE_PTR(SharedStateType);

		protected:
			SharedStateTypePtr			_state;

		public:
			void swap(future_base& other)	{ _state.swap(other._state); }

			bool valid() const				{ return _state.is_initialized(); }
			bool is_ready() const			{ check_valid(); return _state->is_ready(); }
			bool has_exception() const		{ check_valid(); return _state->has_exception(); }
			bool has_value() const			{ check_valid(); return _state->has_value(); }

			future_status wait(const ICancellationToken& token = DummyCancellationToken()) const
			{ check_valid(); return _state->wait(token); }

		protected:
			future_base() { }

			explicit future_base(const SharedStateTypePtr& state) : _state(state) { }

			void check_valid() const { STINGRAYKIT_CHECK(valid(), InvalidFuturePromiseState()); }
		};

	}


	template < typename ResultType >
	class future : public Detail::future_base<ResultType>
	{
		STINGRAYKIT_NONASSIGNABLE(future);

	private:
		using Base = Detail::future_base<ResultType>;

	public:
		future() { }

		shared_future<ResultType> share()
		{
			typename Base::SharedStateTypePtr state(Base::_state);
			Base::_state.reset();
			return shared_future<ResultType>(state);
		}

		ResultType get()
		{
			Base::check_valid();
			typename Base::SharedStateTypePtr tmp;
			tmp.swap(Base::_state);
			return tmp->get();
		}

	private:
		explicit future(const typename Base::SharedStateTypePtr& state) : Base(state) { }
		friend future<ResultType> Detail::promise_base<ResultType>::get_future();
	};


	template < typename ResultType >
	class shared_future : public Detail::future_base<ResultType>
	{
		using Base = Detail::future_base<ResultType>;

	public:
		shared_future() { }

		ResultType get() const			{ Base::check_valid(); return Base::_state->get(); }

	private:
		explicit shared_future(const typename Base::SharedStateTypePtr& state) : Base(state) { }
		friend shared_future<ResultType> future<ResultType>::share();
	};

	/** @} */

}

#endif
