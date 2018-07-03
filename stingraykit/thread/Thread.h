#ifndef STINGRAYKIT_THREAD_THREAD_H
#define STINGRAYKIT_THREAD_THREAD_H

// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

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

	typedef IThread::ThreadId ThreadId;

	class Thread
	{
		STINGRAYKIT_NONCOPYABLE(Thread);

		typedef function<void(const ICancellationToken&)>	FuncType;

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

	public:
		Thread(const std::string& name, const FuncType& threadFunc, optional<TimeDuration> timeout = null);
		~Thread();

		void Interrupt();
		ThreadId GetId();

		static void InterruptionPoint();
		static void Yield();
		/** @deprecated */
		static inline void Sleep(u32 milliseconds)		{ SleepMicroseconds(1000u * (u64)milliseconds); }
		static inline void Sleep(TimeDuration duration)	{ SleepMicroseconds(1000u * duration.GetMilliseconds()); }
		static void SleepMicroseconds(u64 microseconds);
		static ThreadId GetCurrentThreadId();
		static void SetCurrentThreadName(const std::string& name);
		static const std::string& GetCurrentThreadName();
		static IThreadInfoPtr GetCurrentThreadInfo();

		typedef std::vector<ThreadStats> ThreadStatsVec;
		static ThreadStatsVec GetStats();

		static optional<SystemStats> GetSystemStats();
	};
	STINGRAYKIT_DECLARE_PTR(Thread);

	/** @} */

}

#endif
