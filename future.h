#ifndef STINGRAY_TOOLKIT_FUTURE_H
#define STINGRAY_TOOLKIT_FUTURE_H

#include <stingray/threads/ConditionVariable.h>
#include <stingray/threads/Thread.h>
#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/shared_ptr.h>
#include <stingray/toolkit/unique_ptr.h>
#include <stingray/toolkit/toolkit.h>
#include <stingray/toolkit/exception_ptr.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_threads
	 * @{
	 */

	struct future_status
	{
		TOOLKIT_ENUM_VALUES(
			ready,
			timeout
		);
		TOOLKIT_DECLARE_ENUM_CLASS(future_status);
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
		class future_impl
		{
		public:
			typedef typename future_value_holder<ResultType>::ConstructValueT SetType;
		private:
			typedef future_value_holder<ResultType> WrappedResultType;
			typedef unique_ptr<WrappedResultType> ValuePtr;

			Mutex				_mutex;
			ConditionVariable	_condition;
			ValuePtr			_value;
			exception_ptr		_exception;

		public:
			future_impl() {}
			~future_impl() {}

			bool is_ready() const		{ MutexLock l(_mutex); return _value || _exception; }
			bool has_exception() const	{ MutexLock l(_mutex); return _exception; }
			bool has_value() const		{ MutexLock l(_mutex); return _value; }
			void wait()					{ MutexLock l(_mutex); do_wait(); }

			future_status wait_for(const TimeDuration& duration)
			{ MutexLock l(_mutex); return do_timed_wait(duration); }

			future_status wait_until(const Time& absTime)
			{ MutexLock l(_mutex); return do_timed_wait(absTime - Time::Now()); }

			ResultType get()
			{
				MutexLock l(_mutex);
				do_wait();
				if (_exception)
					rethrow_exception(_exception);
				return *_value;
			}

			void set_value(SetType value)
			{
				MutexLock l(_mutex);
				TOOLKIT_CHECK(!_value, PromiseAlreadySatisfied());
				_value.reset(new WrappedResultType(value));
				_condition.Broadcast();
			}

			void set_exception(exception_ptr ex)
			{
				MutexLock l(_mutex);
				if (_value || _exception)
					return;
				_exception = ex;
				_condition.Broadcast();
			}

		private:
			void do_wait()
			{
				if(_value || _exception)
					return;
				_condition.Wait(_mutex);
			}

			future_status do_timed_wait(const TimeDuration& duration)
			{
				if (_value || _exception)
					return future_status::ready;
				_condition.TimedWait(_mutex, duration);
				if (_value || _exception)
					return future_status::ready;
				return future_status::timeout;
			}

		};

		template<>
		class future_impl<void>
		{
		public:
			typedef void SetType;

		private:
			Mutex				_mutex;
			ConditionVariable	_condition;
			bool				_value;
			exception_ptr		_exception;

		public:
			future_impl() : _value(false) {}
			~future_impl() {}

			bool is_ready() const		{ MutexLock l(_mutex); return _value || _exception; }
			bool has_exception() const	{ MutexLock l(_mutex); return _exception; }
			bool has_value() const		{ MutexLock l(_mutex); return _value; }
			void wait()					{ MutexLock l(_mutex); do_wait(); }

			future_status wait_for(const TimeDuration& duration)
			{ MutexLock l(_mutex); return do_timed_wait(duration); }

			future_status wait_until(const Time& absTime)
			{ MutexLock l(_mutex); return do_timed_wait(absTime - Time::Now()); }

			void get()
			{
				MutexLock l(_mutex);
				do_wait();
				if (_exception)
					rethrow_exception(_exception);
			}

			void set_value()
			{
				MutexLock l(_mutex);
				TOOLKIT_CHECK(!_value, PromiseAlreadySatisfied());
				_value = true;
				_condition.Broadcast();
			}

			void set_exception(exception_ptr ex)
			{
				MutexLock l(_mutex);
				if (_value)
					return;
				_exception = ex;
				_condition.Broadcast();
			}

		private:
			void do_wait()
			{
				if(_value || _exception)
					return;
				_condition.Wait(_mutex);
			}

			future_status do_timed_wait(const TimeDuration& duration)
			{
				if (_value || _exception)
					return future_status::ready;
				_condition.TimedWait(_mutex, duration);
				if (_value || _exception)
					return future_status::ready;
				return future_status::timeout;
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

		void wait() const										{ check_valid(); _impl->wait(); }
		future_status wait_for(const TimeDuration& duration)	{ check_valid(); return _impl->wait_for(duration); }
		future_status wait_until(const Time& absTime)			{ check_valid(); return _impl->wait_until(absTime); }
	private:
		shared_future(const ImplPtr& impl) : _impl(impl) {}
		friend shared_future<ResultType> future<ResultType>::share();
		void check_valid() const { TOOLKIT_CHECK(valid(), std::runtime_error("No async result is assigned to the shared_future!")); }
	};

	template<typename ResultType>
	class future
	{
		TOOLKIT_NONASSIGNABLE(future);
	private:
		typedef Detail::future_impl<ResultType> ImplType;
		TOOLKIT_DECLARE_PTR(ImplType);
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
			ResultType result = _impl->get();
			_impl.reset();
			return result;
		}

		void wait() const										{ check_valid(); _impl->wait(); }
		future_status wait_for(const TimeDuration& duration)	{ check_valid(); return _impl->wait_for(duration); }
		future_status wait_until(const Time& absTime)			{ check_valid(); return _impl->wait_until(absTime); }
	private:
		future(const ImplTypePtr& impl) : _impl(impl) {}
		friend future<ResultType> promise<ResultType>::get_future();
		void check_valid() const { TOOLKIT_CHECK(valid(), std::runtime_error("No async result is assigned to the future!")); }
	};

	template<typename ResultType>
	class promise
	{
		TOOLKIT_NONCOPYABLE(promise);

	public:
		typedef typename Detail::future_impl<ResultType>::SetType SetType;

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
		{ TOOLKIT_CHECK(!_futureRetrieved, FutureAlreadyRetrieved()); _futureRetrieved = true; return future<ResultType>(_futureImpl); }

		void set_value(SetType result)
		{ _futureImpl->set_value(result); }

		void set_exception(exception_ptr ex)
		{ _futureImpl->set_exception(ex); }
	};

	template<>
	class promise<void>
	{
		TOOLKIT_NONCOPYABLE(promise);

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
		{ TOOLKIT_CHECK(!_futureRetrieved, FutureAlreadyRetrieved()); _futureRetrieved = true; return future<ResultType>(_futureImpl); }

		void set_value()
		{ _futureImpl->set_value(); }

		void set_exception(exception_ptr ex)
		{ _futureImpl->set_exception(ex); }

	};

	/** @} */

}

#endif
