#ifndef STINGRAYKIT_THREAD_THREAD_H
#define STINGRAYKIT_THREAD_THREAD_H


#define PROFILE_MUTEX_LOCK_TIMINGS 0

#if PROFILE_MUTEX_LOCK_TIMINGS
#	include <stingraykit/thread/LockProfiler.h>
#endif

#if PLATFORM_POSIX
#	include <stingraykit/thread/posix/PosixThreadEngine.h>
	namespace stingray {
		/**
		 * @addtogroup toolkit_threads
		 * @{
		 */
#if PROFILE_MUTEX_LOCK_TIMINGS
		typedef MutexProfilerWrapper<PosixMutex>	Mutex;
#else
		typedef PosixMutex				Mutex;
#endif
		typedef posix::PosixSemaphore	Semaphore;
		typedef PosixThreadEngine		ThreadEngine;
		/** @} */
	}
#elif PLATFORM_OSPLUS
#	include <stingraykit/thread/osplus/OS21ThreadEngine.h>
#if PROFILE_MUTEX_LOCK_TIMINGS
	namespace stingray { typedef MutexProfilerWrapper<OS21Mutex>	Mutex; }
#else
	namespace stingray { typedef OS21Mutex				Mutex; }
#endif
	namespace stingray { typedef OS21Semaphore			Semaphore; }
	namespace stingray { typedef OS21ThreadEngine		ThreadEngine; }
#else
#	error Threads not configured
#endif

/** @} */

#include <stingraykit/thread/GenericMutexLock.h>
#include <stingraykit/thread/GenericSemaphoreLock.h>
#include <stingraykit/thread/ICancellationToken.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_threads
	 * @{
	 */

	class EnableInterruptionPoints
	{
	private:
		bool	_enabled;
		bool	_prevValue;

	public:
		EnableInterruptionPoints(bool value);
		~EnableInterruptionPoints();
	};


	class ExternalAPIGuards
	{
	public:
		struct EnterGuard : public EnterExternalAPIGuardBase
		{ EnterGuard(const char* externalApiEntry) : EnterExternalAPIGuardBase(GetStack(), externalApiEntry) { } };

		struct LeaveGuard : public LeaveExternalAPIGuardBase
		{ LeaveGuard(const char* internalApiEntry) : LeaveExternalAPIGuardBase(GetStack(), internalApiEntry) { } };

	private:
		static ExternalAPIGuardStack* GetStack()
		{
			TLSData* tls = ThreadEngine::GetCurrentThreadData();
			return tls ? &tls->GetExternalAPIGuardStack() : NULL;
		}

	public:
		static std::string GetLocation()
		{
			ExternalAPIGuardStack* stack = GetStack();
			return stack ? stack->Get() : "<no tls>";
		}
	};

	typedef GenericSemaphoreLock<Semaphore>	SemaphoreLock;
	typedef GenericMutexLock<Mutex>			MutexLock;
	typedef GenericMutexUnlock<Mutex>		MutexUnlock;

	////////////////////////////////////////

	class Thread
	{
		STINGRAYKIT_NONCOPYABLE(Thread);

		typedef function<void(const ICancellationToken&)>	FuncType;
		typedef std::vector<ThreadStats>					ThreadStatsVec;

	public:
		class PrioritySetter
		{
			STINGRAYKIT_NONCOPYABLE(PrioritySetter);

		private:
			optional<ThreadSchedulingParams>	_prevParams;

		public:
			PrioritySetter();
			PrioritySetter(ThreadSchedulingParams params);
			~PrioritySetter();

			void Set(ThreadSchedulingParams params);
			void Reset();
		};

	private:
		IThreadPtr		_thread;
		std::string		_name;

	public:
		explicit Thread(const std::string& name, const FuncType& threadFunc);
		~Thread();

		void Interrupt();
		IThread::ThreadId GetId();
		const std::string& GetName() const { return _name; }

		static void InterruptionPoint();
		static void Yield();
		/** @deprecated */
		static inline void Sleep(u32 milliseconds)				{ SleepMicroseconds(1000u * (u64)milliseconds); }
		static inline void Sleep(TimeDuration duration)	{ SleepMicroseconds(1000u * duration.GetMilliseconds()); }
		static void SleepMicroseconds(u64 microseconds);
		static IThread::ThreadId GetCurrentThreadId();
		static void SetCurrentThreadName(const std::string& name);
		static void TrySetCurrentThreadName(const std::string& name);
		static const std::string& GetCurrentThreadName();
		static IThreadInfoPtr GetCurrentThreadInfo();
		static ThreadStatsVec GetStats();
		static void SetCancellationToken(const ICancellationToken& token);
		static void ResetCancellationToken();
		static const ICancellationToken& GetCancellationToken();

	private:
		void BeginThread(const FuncType& threadFunc);
	};
	STINGRAYKIT_DECLARE_PTR(Thread);


	class ThreadCancellationTokenSetter
	{
	public:
		ThreadCancellationTokenSetter(const ICancellationToken& token)
		{ Thread::SetCancellationToken(token); }

		~ThreadCancellationTokenSetter()
		{ Thread::ResetCancellationToken(); }
	};

	/** @} */

}


#endif
