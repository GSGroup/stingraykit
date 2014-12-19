#ifndef STINGRAY_TOOLKIT_THREAD_POSIX_POSIXTHREADENGINE_H
#define STINGRAY_TOOLKIT_THREAD_POSIX_POSIXTHREADENGINE_H

#include <pthread.h>

#include <stingray/toolkit/thread/posix/PosixCallOnce.h>
#include <stingray/toolkit/thread/posix/PosixSemaphore.h>
#include <stingray/toolkit/thread/ICancellationToken.h>
#include <stingray/toolkit/thread/IThreadEngine.h>
#include <stingray/toolkit/diagnostics/Backtrace.h>
#include <stingray/toolkit/function/function.h>

namespace stingray
{


	class PosixMutex
	{
		TOOLKIT_NONCOPYABLE(PosixMutex);

		friend class PosixConditionVariable;

	private:
		mutable pthread_mutex_t		_rawMutex;
		void HandleReturnCode(const char *where, int code) const; //throws system exception

	public:
		PosixMutex();

		inline ~PosixMutex()
		{ pthread_mutex_destroy(&_rawMutex); }

		void Lock() const;
		void Unlock() const;
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
		static void TrySetCurrentThreadName(const std::string& name);
		static ThreadStatsVec GetStingrayThreadsStats();
		static ThreadStatsVec GetThreadsStats();

		static ThreadSchedulingParams SetCurrentThreadPriority(ThreadSchedulingParams params);

		static void CallOnce(OnceNativeType& once, void (*func)()) { PosixCallOnce::CallOnce(once, func); }

	private:
		static void* ThreadFunc(void* arg);
	};


}


#endif
