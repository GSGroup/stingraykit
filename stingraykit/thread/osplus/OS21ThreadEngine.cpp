// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <os21/st40/context.h>
#include <os21/st40/osdefs.h>

#ifdef HAVE_ABI_FORCE_UNWIND
#include <cxxabi.h>
#endif

#include <memory>
#include <string>

#include <stingraykit/exception.h>
#include <stingraykit/log/Logger.h>
#include <stingraykit/string/ToString.h>
#include <stingraykit/thread/GenericMutexLock.h>
#include <stingraykit/thread/atomic/AtomicInt.h>
#include <stingraykit/thread/osplus/OS21TaskPrivate.h>
#include <stingraykit/thread/osplus/OS21ThreadEngine.h>


namespace stingray
{




	namespace
	{

		struct OS21TLSUserData : public ITLSUserData
		{
			static AtomicU32::Type	s_threadIdGenerator;

			u32						ThreadId;
			bool					InterruptRequired;
			int						DisableInterruptionPoints;
			OS21Mutex				Mutex;

			OS21TLSUserData(task_t* taskPtr)
				: ThreadId(AtomicU32::Inc(s_threadIdGenerator)),
				  InterruptRequired(false),
				  DisableInterruptionPoints(0)
			{ }

			STINGRAYKIT_NONCOPYABLE(OS21TLSUserData);
		};

		AtomicU32::Type OS21TLSUserData::s_threadIdGenerator = 0;


		struct TLSDataHolder
		{
			TLSData	tlsData;

			TLSDataHolder(const TLSData& tlsData)
				: tlsData(tlsData)
			{ }

			STINGRAYKIT_NONCOPYABLE(TLSDataHolder);
		};


		class TLS
		{
		public:
			static void Init(const TLSData& tlsData)
			{
				task_t *task = task_id();
				STINGRAYKIT_CHECK(!IsInitialized(task), std::runtime_error("TLS data already assigned!"));
				Initialize(NULL, TLSData(tlsData.GetThreadName(), ITLSUserDataPtr(new OS21TLSUserData(task))));
			}

			static bool IsInitialized(task_t* id) { return task_private_data(id, NULL); }

			static TLSData* Get(task_t* taskPtr)
			{
				if (!IsInitialized(taskPtr))
					Initialize(taskPtr, TLSData(Thread::UndefinedThreadName, ITLSUserDataPtr(new OS21TLSUserData(taskPtr))));

				TLSDataHolder* holder = static_cast<TLSDataHolder*>(task_private_data(taskPtr, NULL));
				return holder ? &holder->tlsData : NULL;
			}

		private:
			static void Initialize(task_t* taskPtr, const TLSData& tlsData)
			{
				std::auto_ptr<TLSDataHolder> tls_ptr(new TLSDataHolder(tlsData));
				STINGRAYKIT_CHECK(task_private_data_set(taskPtr, tls_ptr.get(), NULL, &TLS::Dtor) == OS21_SUCCESS, Exception("task_private_data_set"));
				tls_ptr.release();
			}

			static void Dtor(void *data)
			{
				TLSDataHolder* holder = static_cast<TLSDataHolder*>(data);
				delete holder;
			}
		};


		struct TLSDataGetter
		{
			TLSData*			Data;
			OS21TLSUserData*	UserData;

			TLSDataGetter(task_t* task = NULL)
			{
				Data = TLS::Get(task);
				STINGRAYKIT_CHECK(Data, Exception("No TLS data!"));
				UserData = static_cast<OS21TLSUserData*>(Data->GetUserData().get());
				STINGRAYKIT_CHECK(UserData, Exception("No TLS user data!"));
				UserData->Mutex.Lock();
			}

			~TLSDataGetter()
			{ UserData->Mutex.Unlock(); }
		};


	}


#if CHECK_MUTEX_DEADLOCK
	void OS21Mutex::NotifyAboutDeadlock(task_t* lockedBy, task_t* tryLockBy)
	{
		Backtrace backtrace;
		Logger::Warning() << "Could not lock mutex for 3 s (locked by " << lockedBy->task_name << ", try to lock by " << tryLockBy->task_name << "), there is probably a deadlock\n" << backtrace.Get();
	}
#endif


	struct InterruptException
	{ static void Throw() { throw InterruptException(); } };


	class OS21Thread : public virtual IThread
	{
	private:
		task_t*		_thread;
		bool		_threadIsDetached;

	public:
		OS21Thread():
			_thread(0),
			_threadIsDetached(false)
		{ }

		virtual ~OS21Thread() { Join() }

		void SetThread(task_t *thread) { _thread = thread; }

		void DetachThread() { _threadIsDetached = true; }

		void Join()
		{
			if (_threadIsDetached)
				return;

			STINGRAYKIT_CHECK(task_wait(&_thread, 1, TIMEOUT_INFINITY) == OS21_SUCCESS, std::runtime_error("task_wait"));
			STINGRAYKIT_CHECK(task_delete(_thread) == OS21_SUCCESS, std::runtime_error("task_delete"));
		}

		virtual void Interrupt()
		{
			if (_threadIsDetached)
				return;

			TLSDataGetter tls_data(_thread);
			tls_data.UserData->InterruptRequired = true;
		}

		virtual ThreadId GetId() { return (ThreadId)_thread; }
	};
	STINGRAYKIT_DECLARE_PTR(OS21Thread);


	class ThreadArg
	{
	private:
		typedef function<void()>	FuncType;

		FuncType			_func;
		OS21ThreadWeakPtr	_threadObj;
		TLSData				_tlsData;

	public:
		ThreadArg(const FuncType& func, const OS21ThreadWeakPtr& threadObj, const TLSData& tlsData):
			_func(func),
			_threadObj(threadObj),
			_tlsData(tlsData)
		{ }

		const FuncType& GetFunc() const				{ return _func; }
		OS21ThreadWeakPtr GetThreadObject() const	{ return _threadObj; }
		const TLSData& GetTLSData() const			{ return _tlsData; }
	};




	void OS21ThreadEngine::ThreadFunc(void* arg)
	{
		class ThreadDetacher
		{
			OS21ThreadWeakPtr	_os21Thread;

		public:
			ThreadDetacher(const OS21ThreadWeakPtr& os21Thread) : _os21Thread(os21Thread) { }

			~ThreadDetacher()
			{
				OS21ThreadPtr locked_thread_obj = _os21Thread.lock();
				if (locked_thread_obj != NULL)
					locked_thread_obj->DetachThread();
			}
		};

		class ThreadFuncLogger
		{
		public:
			ThreadFuncLogger()
			{ }

			void LogEnter()
			{
				TLSDataGetter tls_data;
				Logger::Info() << "[OS21ThreadEngine] Entered threadfunc of thread " << tls_data.Data->GetThreadName() << " with id = " << tls_data.UserData->ThreadId;
			}

			~ThreadFuncLogger()
			{
				TLSDataGetter tls_data;

				if (std::uncaught_exception())
					Logger::Info() << "[OS21ThreadEngine] Exiting threadfunc of thread " << tls_data.Data->GetThreadName() << " with id = " << tls_data.UserData->ThreadId << " due to an exception";
				else
					Logger::Info() << "[OS21ThreadEngine] Exiting threadfunc of thread " << tls_data.Data->GetThreadName() << " with id = " << tls_data.UserData->ThreadId;
			}

			u32 GetId() const
			{
				TLSDataGetter tls_user_data;
				return tls_user_data.UserData->ThreadId;
			}
		};

		std::auto_ptr<ThreadArg> thread_arg(static_cast<ThreadArg*>(arg));

		try
		{
			TLS::Init(thread_arg->GetTLSData());
		}
		catch (...)
		{
			Logger::Error() << "[OS21ThreadEngine] TLS::Init() failed for task named '" << thread_arg->GetTLSData().GetThreadName() << "'";
			throw;
		}

		ThreadFuncLogger tfl;

		try
		{
			ThreadDetacher detacher(thread_arg->GetThreadObject());

			tfl.LogEnter(); // Should be performed after TLS initialization

			(thread_arg->GetFunc())(); // Execute!
		}
		catch (const std::exception& ex)
		{
			Logger::Error() << "[OS21ThreadEngine] Uncaught std::exception:" << ex << "\n\tin thread with tid = " << tfl.GetId();
		}
#ifdef HAVE_ABI_FORCE_UNWIND
		catch (abi::__forced_unwind&)
		{
			Logger::Debug() << "[OS21ThreadEngine] Thread with id = " << tfl.GetId() << " interrupted";
			throw;
		}
#endif
		catch (const InterruptException&)
		{
			Logger::Info() << "[OS21ThreadEngine] Thread with id = " << tfl.GetId() << " interrupted";
		}
		catch (...)
		{
			Logger::Error() << "[OS21ThreadEngine] Uncaught unknown exception\n\tin thread with id = " << tfl.GetId();
			throw;
		}
	}

	namespace
	{
		static const int DefaultTaskPriority = MAX_USER_PRIORITY - 15; //lowest reserved priority, MIN is too low
	}

	IThreadPtr OS21ThreadEngine::BeginThread(const FuncType& func, const std::string& name)
	{
		shared_ptr<OS21Thread> thread_obj(new OS21Thread);
		TLSData tls_data(name);
		std::auto_ptr<ThreadArg> thread_arg(new ThreadArg(func, thread_obj, tls_data));

		int priority = DefaultTaskPriority;
		if (name == "inputDevice")
			priority = MAX_USER_PRIORITY;
		if (name == "ciLinkConnection")
			++priority;

		Logger::Info() << "Thread " << name << " priority: " << priority;
		task_t *thread = task_create(&OS21ThreadEngine::ThreadFunc, thread_arg.get(), OS21_DEF_SYSTEM_STACK_SIZE, priority, name.c_str(), task_flags_suspended);

		if (name == "mallocLogger")
		{
			Logger::Info() << "boosting mallocLogger task priority";
			int r = task_priority_set(thread, DefaultTaskPriority + 15);
			Logger::Info() << "task_priority_set returned " << r;
		}

		thread_obj->SetThread(thread);
		task_resume(thread);

		STINGRAYKIT_CHECK(thread, std::runtime_error("task_create failed!"));

		thread_arg.release();

		return thread_obj;
	}

	void OS21ThreadEngine::Yield()
	{
		task_yield();
	}

	void OS21ThreadEngine::SleepMicroseconds(u64 microseconds)
	{
		task_delay((microseconds * time_ticks_per_sec() + 999999) / 1000000); // at least one
	}

	void OS21ThreadEngine::InterruptionPoint()
	{
		TLSDataGetter tls_data;

		if (tls_data.UserData->DisableInterruptionPoints <= 0 && tls_data.UserData->InterruptRequired)
			InterruptException::Throw();
	}

	bool OS21ThreadEngine::EnableInterruptionPoints(bool enable)
	{
		TLSDataGetter tls_data;

		tls_data.UserData->DisableInterruptionPoints += enable ? -1 : 1;

		return !enable;
	}

	IThread::ThreadId OS21ThreadEngine::GetCurrentThreadId()
	{ return task_id(); }

	TLSData* OS21ThreadEngine::GetCurrentThreadData()
	{
		return TLS::Get(task_id());
	}

	void OS21ThreadEngine::SetCurrentThreadName(const std::string& name)
	{
		task_t *task = task_id();
		if (name == "ui")
		{
			Logger::Info() << "boosting ui task priority";
			int r = task_priority_set(task, DefaultTaskPriority);
			Logger::Info() << "task_priority_set returned " << r;
		}
		TLSData* tls_data = TLS::Get(task);
		OS21TLSUserData* tls_udata = tls_data ? static_cast<OS21TLSUserData*>(tls_data->GetUserData().get()) : NULL;

		MutexLock l(tls_udata->Mutex);
		tls_data->SetThreadName(name);
	}

	OS21ThreadEngine::ThreadStatsVec OS21ThreadEngine::GetThreadsStats()
	{
		ScopedTaskLock l; // prevents current task from preemption

		ThreadStatsVec result;
		for (task_t * task_p = task_list_next(NULL); task_p != NULL; task_p = task_list_next(task_p))
		{
			task_status_t task_stat = {};
			STINGRAYKIT_CHECK(task_status(task_p, &task_stat, 0) == OS21_SUCCESS, Exception("task_status failed!"));
			u64 systemTime = ((u64)task_stat.task_time * 1000) / time_ticks_per_sec();

			if (TLS::IsInitialized(task_p))
			{
				TLSDataGetter tls_data(task_p);
				result.push_back(ThreadStats(tls_data.UserData->ThreadId, 0, tls_data.Data->GetThreadName(), 0, systemTime, 0, 0));
			}
			else
			{
				result.push_back(ThreadStats((u32)task_p, 0, task_name(task_p), 0, systemTime, 0, 0));
			}
		}

		return result;
	}

	s64 OS21ThreadEngine::GetThreadMicroseconds()
	{
		task_status_t task_stat;
		task_t* current_task = task_id();
		if (!current_task) // os21 performs some mallocs from its early initialization. =(
			return 0;
		if (task_status(current_task, &task_stat, 0) != OS21_SUCCESS)
			return 0; // Whatever. Do not want any allocations to happen.
		return ((s64)task_stat.task_time * 1000000) / time_ticks_per_sec();
	}


	ThreadSchedulingParams OS21ThreadEngine::SetCurrentThreadPriority(ThreadSchedulingParams params)
	{
		if (params.GetPolicy() != ThreadSchedulingPolicy::NonRealtime)
			Logger::Warning() << "Attempt to set RealTime scheduling policy under OS21";

		task_priority_set(task_id(), priority);
		return ThreadSchedulingParams(ThreadSchedulingPolicy::NonRealtime, DefaultTaskPriority); // TODO: get actual params
	}


	std::string OS21ThreadEngine::GetThreadNameByTaskTPtr(task_t* task_p)
	{
		TLSDataGetter tls_data(task_p);
		return tls_data.Data->GetThreadName();
	}


}
