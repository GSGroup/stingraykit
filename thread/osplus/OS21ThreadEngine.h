#ifndef STINGRAY_TOOLKIT_THREAD_OSPLUS_OS21THREADENGINE_H
#define STINGRAY_TOOLKIT_THREAD_OSPLUS_OS21THREADENGINE_H


#include <os21/mutex.h>
#include <os21/ostime.h>
#include <os21/semaphore.h>
#include <os21/task.h>

#include <stingray/toolkit/thread/osplus/OS21CallOnce.h>
#include <stingray/toolkit/thread/IThreadEngine.h>
#include <stingray/toolkit/time/Time.h>
#include <stingray/toolkit/function/function.h>


#define CHECK_MUTEX_DEADLOCK 1


namespace stingray
{

#if CHECK_MUTEX_DEADLOCK
	class OS21Mutex
	{
		STINGRAYKIT_NONCOPYABLE(OS21Mutex);

		class ScopedMutexLock
		{
		private:
			mutex_t*	_mutex;

		public:
			explicit ScopedMutexLock(mutex_t* mutex)
				: _mutex(mutex)
			{ mutex_lock(_mutex); }

			~ScopedMutexLock()
			{ mutex_release(_mutex); }
		};

	private:
		semaphore_t*	_sp;
		mutex_t*		_mutex;

		task_t*			_task;
		u32				_count;

	public:
		OS21Mutex() : _sp(semaphore_create_fifo(1)), _mutex(mutex_create_fifo()), _task(0), _count(0)
		{
			STINGRAYKIT_CHECK(_sp, std::runtime_error("semaphore_create_fifo"));
			STINGRAYKIT_CHECK(_mutex, std::runtime_error("mutex_create_priority"));
		}

		~OS21Mutex()
		{
			semaphore_delete(_sp);
			mutex_delete(_mutex);
		}

		void Lock() const
		{
			mutex_lock(_mutex);

			if (_task != task_id())
			{
				mutex_release(_mutex);

				osclock_t relative = time_ticks_per_sec() * 3;
				osclock_t absolute = time_plus(time_now(), relative);
				if (semaphore_wait_timeout(_sp, &absolute) != OS21_SUCCESS)
				{
					{
						ScopedMutexLock l(_mutex);
						if (_task)
							NotifyAboutDeadlock(_task, task_id());
					}

					semaphore_wait(_sp);
				}

				mutex_lock(_mutex);

				if (_task)
					STINGRAYKIT_FATAL("Task not null!");

				if (_count)
					STINGRAYKIT_FATAL("Count not zero!");

				_task = task_id();
			}

			++_count;

			mutex_release(_mutex);
		}

		void Unlock() const
		{
			ScopedMutexLock l(_mutex);

			if (--_count <= 0)
			{
				_task = 0;
				_count = 0;
				semaphore_signal(_sp);
			}
		}

	private:
		void NotifyAboutDeadlock(task_t* lockedBy, task_t* tryLockBy);
	};

#else
	class OS21Mutex
	{
		STINGRAYKIT_NONCOPYABLE(OS21Mutex);

	private:
		mutex_t*	_mutex;

	public:
		OS21Mutex()
		{ STINGRAYKIT_CHECK((_mutex = mutex_create_priority()), std::runtime_error("mutex_create_priority")); }

		~OS21Mutex()
		{ mutex_delete(_mutex); }

		inline void Lock()
		{ mutex_lock(_mutex); }

		inline bool TryLock()
		{ return (OS21_SUCCESS == mutex_trylock(_mutex)); }

		inline void Unlock()
		{ mutex_release(_mutex); }
	};
#endif

	class OS21Semaphore
	{
		STINGRAYKIT_NONCOPYABLE(OS21Semaphore);

	private:
		semaphore_t*	_sp;

	public:
		OS21Semaphore(int count = 0)
		{ STINGRAYKIT_CHECK((_sp = semaphore_create_priority(count)), std::runtime_error("semaphore_create_priority")); }

		~OS21Semaphore()
		{ semaphore_delete(_sp); }

		inline void Wait() const
		{ STINGRAYKIT_CHECK(OS21_SUCCESS == semaphore_wait(_sp), std::runtime_error("semaphore_wait failed!")); }

		// TODO: implement TimedWait(absTime) version
		inline bool TimedWait(TimeDuration timeout) const
		{
			osclock_t relative = (time_ticks_per_sec() * timeout.GetMilliseconds()) / 1000;
			osclock_t absolute = time_plus(time_now(), relative);
			return semaphore_wait_timeout(_sp, &absolute) == OS21_SUCCESS;
		}

		inline void Signal() const
		{ semaphore_signal(_sp); }

		inline void Signal(int n) const
		{ while(n-- > 0) semaphore_signal(_sp); }
	};


	class OS21ThreadEngine
	{
		class ScopedTaskLock
		{
		public:
			ScopedTaskLock()
			{ task_lock(); }

			~ScopedTaskLock()
			{ task_unlock(); }
		};

	public:
		typedef std::vector<ThreadStats>		ThreadStatsVec;
		typedef function<void()>				FuncType;
		typedef OS21CallOnce::OnceNativeType	OnceNativeType;

	public:
		static IThreadPtr BeginThread(const FuncType& func, const std::string& name);
		static void Yield();
		static void Sleep(u32 milliseconds)
		{ SleepMicroseconds(1000u * (u64)milliseconds); }
		static void SleepMicroseconds(u64 microseconds);
		static void InterruptionPoint();
		static bool EnableInterruptionPoints(bool enable);
		static TLSData* GetCurrentThreadData();
		static void SetCurrentThreadName(const std::string& name);
		static void TrySetCurrentThreadName(const std::string& name);
		static ThreadStatsVec GetThreadsStats();
		static void CallOnce(OnceNativeType& once, void (*func)()) { OS21CallOnce::CallOnce(once, func); }
		static s64 GetThreadMicroseconds();

		static ThreadSchedulingParams SetCurrentThreadPriority(ThreadSchedulingParams params);

		static std::string GetThreadNameByTaskTPtr(task_t* task_p);

	private:
		static void ThreadFunc(void* arg);
	};

}


#endif
