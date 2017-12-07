#ifndef STINGRAYKIT_THREAD_POSIX_POSIXTHREADENGINE_H
#define STINGRAYKIT_THREAD_POSIX_POSIXTHREADENGINE_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

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
		static IThreadPtr BeginThread(const FuncType& func, const std::string& name, optional<TimeDuration> timeout = null);
		static void Yield();
		static inline void Sleep(u32 milliseconds)
		{ SleepMicroseconds(1000u * (u64)milliseconds); }
		static void SleepMicroseconds(u64 microseconds);
		static void InterruptionPoint();
		static bool EnableInterruptionPoints(bool enable);
		static IThread::ThreadId GetCurrentThreadId();
		static pthread_t GetCurrentPthreadId();
		static pthread_t GetPthreadIdFromThreadId(IThread::ThreadId tid);
		static IThreadInfoPtr GetCurrentThreadInfo();
		static TLSData* GetCurrentThreadData();

		static void SetCurrentThreadName(const std::string& name);
		static const std::string& GetCurrentThreadName();

		static ThreadStatsVec GetThreadsStats();

		static optional<SystemStats> GetSystemStats();

		static ThreadSchedulingParams SetCurrentThreadPriority(ThreadSchedulingParams params);

		static void CallOnce(OnceNativeType& once, void (*func)()) { PosixCallOnce::CallOnce(once, func); }
	};


}


#endif
