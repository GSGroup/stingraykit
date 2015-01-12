#ifndef STINGRAY_TOOLKIT_TIMER_TIMER_H
#define STINGRAY_TOOLKIT_TIMER_TIMER_H


#include <stingray/toolkit/Final.h>
#include <stingray/toolkit/ScopeExit.h>
#include <stingray/toolkit/function/bind.h>
#include <stingray/toolkit/function/function.h>
#include <stingray/toolkit/log/Logger.h>
#include <stingray/toolkit/thread/ConditionVariable.h>
#include <stingray/toolkit/thread/ITaskExecutor.h>
#include <stingray/toolkit/thread/Thread.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_timer
	 * @{
	 */

	namespace Detail
	{
		struct ITimerConnectionImpl
		{
			virtual ~ITimerConnectionImpl() { }

			virtual void Disconnect() = 0;
		};
		TOOLKIT_DECLARE_PTR(ITimerConnectionImpl);
	}


	class TimerConnectionToken : public virtual IToken
	{
		friend class Timer;

	private:
		Detail::ITimerConnectionImplPtr		_impl;

		TimerConnectionToken(const Detail::ITimerConnectionImplPtr& impl)
			: _impl(impl)
		{ }

	public:
		~TimerConnectionToken()
		{
			if (_impl)
			{
				_impl->Disconnect();
				_impl.reset();
			}
		}
	};


	class Timer : TOOLKIT_FINAL(Timer), public virtual ITaskExecutor
	{
		TOOLKIT_NONCOPYABLE(Timer);

		static NamedLogger			s_logger;

		class CallbackInfo;
		TOOLKIT_DECLARE_PTR(CallbackInfo);

		class CallbackQueue;
		TOOLKIT_DECLARE_PTR(CallbackQueue);

		struct CallbackInfoOrder;

	public:
		typedef function<void(const std::exception&)>	ExceptionHandler;

	private:
		std::string					_timerName;
		ThreadPtr					_worker;
		ConditionVariable			_cond;
		bool						_working;
		bool						_alive;
		CallbackQueuePtr			_queue;
		ExceptionHandler			_exceptionHandler;
		bool						_profileCalls;

	public:
		Timer(const std::string& timerName, const ExceptionHandler& exceptionHandler = &Timer::DefaultExceptionHandler, bool profileCalls = true);
		~Timer();

		void Shutdown();

		void Start();
		void Stop();

		ITokenPtr SetTimeout(const TimeDuration& timeout, const function<void()>& func);
		ITokenPtr SetTimer(const TimeDuration& interval, const function<void()>& func);
		ITokenPtr SetTimer(const TimeDuration& timeout, const TimeDuration& interval, const function<void()>& func);

		virtual void AddTask(const function<void()>& task)
		{ AddTask(task, null); }

		virtual void AddTask(const function<void()>& task, const FutureExecutionTester& tester);

		/** @deprecated */
		ITokenPtr SetTimeout(size_t timeoutMilliseconds, const function<void()>& func)
		{ return SetTimeout(TimeDuration(timeoutMilliseconds), func); }

		/** @deprecated */
		ITokenPtr SetTimer(size_t intervalMilliseconds, const function<void()>& func)
		{ return SetTimer(TimeDuration(intervalMilliseconds), func); }

		/** @deprecated */
		ITokenPtr SetTimer(size_t timeoutMilliseconds, size_t intervalMilliseconds, const function<void()>& func)
		{ return SetTimer(TimeDuration(timeoutMilliseconds), TimeDuration(intervalMilliseconds), func); }

		static void DefaultExceptionHandler(const std::exception& ex);

	private:
		std::string GetProfilerMessage(const function<void()>& func);
		void ThreadFunc();
	};
	TOOLKIT_DECLARE_PTR(Timer);


	class ExecutionDeferrer
	{
	private:
		size_t						_timeout;
		Timer&						_timer;

		ITokenPtr					_connection;
		Mutex						_connectionMutex;

		ITokenPtr					_doDeferConnection;
		Mutex 						_doDeferConnectionMutex;

		Mutex						_mutex;
		bool						_cancellationActive;

	public:
		explicit ExecutionDeferrer(Timer& timer, size_t timeoutMilliseconds = 0)
			: _timeout(timeoutMilliseconds), _timer(timer), _cancellationActive(false)
		{ }

		~ExecutionDeferrer()
		{ Cancel(); }

		size_t GetTimeout() const		{ return _timeout; }
		void SetTimeout(size_t timeout)	{ _timeout = timeout; }
		void Cancel()
		{
			{
				MutexLock l(_mutex);
				if (IsCancellationActive())
					return;
				SetCancellationState(true);
			}

			ScopeExitInvoker sei(bind(&ExecutionDeferrer::SetCancellationState, this, false));

			{
				MutexLock l(_doDeferConnectionMutex);
				_doDeferConnection.reset();
			}
			{
				MutexLock l(_connectionMutex);
				_connection.reset();
			}
		}

		// we shouldn't call Defer from deferred function!
		void Defer(const function<void()>& func)
		{
			TOOLKIT_CHECK(_timeout != 0, Exception("Invalid timeout!"));
			Defer(func, _timeout);
		}

		// custom timeout version - doesn't change "default" timeout value stored in deferrer - passed timeout value corresponds to the very deferring
		void Defer(const function<void()>& func, size_t timeout, optional<size_t> interval = null)
		{
			MutexLock l(_doDeferConnectionMutex);
			_doDeferConnection = _timer.SetTimeout(0, bind(&ExecutionDeferrer::DoDefer, this, func, timeout, interval));
		}

		void DeferNoTimeout(const function<void()>& func)					{ Defer(func); }
		void DeferWithTimeout(const function<void()>& func, size_t timeout)	{ Defer(func, timeout); }

	private:
		void DoDefer(const function<void()>& func, size_t timeout, optional<size_t> interval)
		{
			MutexLock l(_connectionMutex);
			if (interval)
				_connection = _timer.SetTimer(timeout, *interval, func);
			else
				_connection = _timer.SetTimeout(timeout, func);
		}

		void SetCancellationState(bool active)
		{ MutexLock l(_mutex); _cancellationActive = active; }

		bool IsCancellationActive() const
		{ MutexLock l(_mutex); return _cancellationActive; }
	};
	TOOLKIT_DECLARE_PTR(ExecutionDeferrer);


	class ExecutionDeferrerWithTimer
	{
	private:
		Timer					_timer;
		ExecutionDeferrerPtr	_impl;

	public:
		explicit ExecutionDeferrerWithTimer(const std::string& timerName, size_t timeout = 0)
			: _timer(timerName)
		{ _impl.reset(new ExecutionDeferrer(_timer, timeout)); }

		~ExecutionDeferrerWithTimer()
		{ _impl.reset(); }

		size_t GetTimeout() const											{ return _impl->GetTimeout(); }
		void SetTimeout(size_t timeout)										{ _impl->SetTimeout(timeout); }
		void Cancel()														{ _impl->Cancel(); }
		void Defer(const function<void()>& func)							{ _impl->Defer(func); }
		void Defer(const function<void()>& func, size_t timeout)			{ _impl->Defer(func, timeout); }
		void DeferNoTimeout(const function<void()>& func)					{ Defer(func); }
		void DeferWithTimeout(const function<void()>& func, size_t timeout)	{ Defer(func, timeout); }
	};
	TOOLKIT_DECLARE_PTR(ExecutionDeferrerWithTimer);

	/** @} */

}

#endif
