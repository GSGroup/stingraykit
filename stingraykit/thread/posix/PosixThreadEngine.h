#ifndef STINGRAYKIT_THREAD_POSIX_POSIXTHREADENGINE_H
#define STINGRAYKIT_THREAD_POSIX_POSIXTHREADENGINE_H

#include <pthread.h>

#include <stingraykit/thread/posix/PosixCallOnce.h>
#include <stingraykit/thread/posix/PosixSemaphore.h>
#include <stingraykit/thread/ICancellationToken.h>
#include <stingraykit/thread/IThreadEngine.h>
#include <stingraykit/diagnostics/Backtrace.h>
#include <stingraykit/function/function.h>

namespace stingray
{


	class PosixMutex
	{
		STINGRAYKIT_NONCOPYABLE(PosixMutex);

		friend class PosixConditionVariable;

	private:
		mutable pthread_mutex_t		_rawMutex;
		void HandleReturnCode(const char *where, int code) const; //throws system exception

	public:
		PosixMutex();

		inline ~PosixMutex()
		{ pthread_mutex_destroy(&_rawMutex); }

		void Lock() const
		{
			int result = pthread_mutex_trylock(&_rawMutex);
			if (result == 0)
				return;
			DoLock(result);
		}

		void Unlock() const
		{
			int result = pthread_mutex_unlock(&_rawMutex);
			if (STINGRAYKIT_UNLIKELY(result != 0))
				HandleReturnCode("pthread_mutex_unlock", result);
		}

	private:
		void DoLock(int tryLockResult) const;
	};


	class PosixThreadEngine
	{
	public:
		typedef std::vector<ThreadStats>					ThreadStatsVec;
		typedef function<void(const ICancellationToken&)>	FuncType;
		typedef PosixCallOnce::OnceNativeType				OnceNativeType;

	public:
		static IThreadPtr BeginThread(const FuncType& func, const std::string& name);
		static void Yield();
		static inline void Sleep(u32 milliseconds)
		{ SleepMicroseconds(1000u * (u64)milliseconds); }
		static void SleepMicroseconds(u64 microseconds);
		static void InterruptionPoint();
		static bool EnableInterruptionPoints(bool enable);
		static IThread::ThreadId GetCurrentThreadId();
		static IThreadInfoPtr GetCurrentThreadInfo();
		static TLSData* GetCurrentThreadData();

		static void SetCurrentThreadName(const std::string& name);
		static const std::string& GetCurrentThreadName();

		static ThreadStatsVec GetThreadsStats();

		static ThreadSchedulingParams SetCurrentThreadPriority(ThreadSchedulingParams params);

		static void CallOnce(OnceNativeType& once, void (*func)()) { PosixCallOnce::CallOnce(once, func); }
	};


}


#endif
