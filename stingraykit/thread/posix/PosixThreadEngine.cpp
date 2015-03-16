// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/posix/PosixThreadEngine.h>

#include <stingraykit/Holder.h>
#include <stingraykit/Mapper.h>
#include <stingraykit/ScopeExit.h>
#include <stingraykit/Singleton.h>
#include <stingraykit/SystemException.h>
#include <stingraykit/diagnostics/ExecutorsProfiler.h>
#include <stingraykit/exception.h>
#include <stingraykit/fatal.h>
#include <stingraykit/function/bind.h>
#include <stingraykit/log/Logger.h>
#include <stingraykit/optional.h>
#include <stingraykit/string/Hex.h>
#include <stingraykit/string/StringFormat.h>
#include <stingraykit/string/ToString.h>
#include <stingraykit/string/string_stream.h>
#include <stingraykit/task_alive_token.h>
#include <stingraykit/thread/CancellationToken.h>
#include <stingraykit/thread/DummyCancellationToken.h>
#include <stingraykit/thread/GenericMutexLock.h>
#include <stingraykit/thread/posix/PosixThreadStats.h>
#include <stingraykit/thread/posix/SignalHandler.h>
#include <stingraykit/thread/posix/ThreadLocal.h>
#include <stingraykit/time/posix/utils.h>
#include <stingraykit/unique_ptr.h>

#include <stdio.h>

#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef HAVE_ABI_FORCE_UNWIND
#include <cxxabi.h>
#endif

#include <memory>

namespace stingray
{

	static unsigned ThreadDtorProfileReportThreshold = 3000;

	static u64 gettid() { return (u64)syscall(SYS_gettid); }


	static NamedLogger PTELogger("PosixThreadEngine");

	struct ThreadBacktracePrinter
	{

		//please use only signal safe function here.
		//http://pubs.opengroup.org/onlinepubs/9699919799/functions/V2_chap02.html#tag_15_04
		//note that malloc not on the list, but it seems to be ok.

		static void Print(const std::string &str)
		{ ssize_t r = write(STDERR_FILENO, str.c_str(), str.size()); (void)r; }

		static void Print(const char * str)
		{
			size_t size = 0;
			const char *p = str;
			while(*p++)
					++size;

			ssize_t r = write(STDERR_FILENO, str, size);
			(void)r;
		}

		static void Print(char c)
		{ ssize_t r = write(STDERR_FILENO, &c, 1); (void)r; }

		static void HandlerFunc(int signalNum, siginfo_t* sigInfo, void* ctx)
		{
			try
			{
				PTELogger.Warning() << "Backtrace of thread " << Thread::GetCurrentThreadName() <<  ": " << Backtrace();
			}
			catch (const std::exception& ex)
			{
				try
				{ Print(diagnostic_information(ex)); Print('\n'); }
				catch(const std::exception&)
				{ Print(ex.what()); Print('\n'); }
			}
			catch (...)
			{
				Print("PosixThreadEngine: unknown exception while logging backtrace of the threads\n");
				throw;
			}
		}
	};


	static posix::SignalHandlerSetter<ThreadBacktracePrinter>		g_threadBacktracePrinter(SIGUSR1);


	class PosixThreadInfo : public virtual IThreadInfo
	{
	private:
		pthread_t					_threadHandle;
		std::string					_name;
		FutureExecutionTester		_threadGuard;

	public:
		PosixThreadInfo(const pthread_t& threadHandle, const std::string& name, const FutureExecutionTester& threadGuard) :
			_threadHandle(threadHandle), _name(name), _threadGuard(threadGuard)
		{ }

		virtual void RequestBacktrace() const
		{
			LocalExecutionGuard g(_threadGuard);
			if (g)
				posix::SendSignal(_threadHandle, g_threadBacktracePrinter.GetSignalNum());
		}

		virtual std::string GetName() const { return _name; }
	};
	STINGRAYKIT_DECLARE_PTR(PosixThreadInfo);


	class ThreadDataStorage;
	STINGRAYKIT_DECLARE_PTR(ThreadDataStorage);


	class ThreadDataStorage
	{
		const u64					_kernelId;
		const pthread_t				_pthreadId;
		const ThreadDataStoragePtr	_parent;

		const PosixThreadInfoPtr	_threadInfo;

		Mutex						_mutex;
		std::string					_name;
		ThreadCpuStats				_childrenStats;

	public:
		ThreadDataStorage(u64 kernelId, pthread_t pthreadId, const std::string& name, const ThreadDataStoragePtr& parent, const FutureExecutionTester& executionTester) :
			_kernelId(kernelId),
			_pthreadId(pthreadId),
			_parent(parent),
			_threadInfo(make_shared<PosixThreadInfo>(pthreadId, name, executionTester)),
			_name(name)
		{ }

		u64 GetKernelId() const										{ return _kernelId; }
		pthread_t GetPthreadId() const								{ return _pthreadId; }
		PosixThreadInfoPtr GetThreadInfo() const					{ return _threadInfo; }
		ThreadDataStoragePtr GetParentThread() const				{ return _parent; }

		std::string GetThreadName() const							{ GenericMutexLock<PosixMutex> l(_mutex); return _name; }
		void SetThreadName(const std::string& name)					{ GenericMutexLock<PosixMutex> l(_mutex); _name = name; }

		void StoreChildStats(ThreadCpuStats childStats) 			{ GenericMutexLock<PosixMutex> l(_mutex); _childrenStats += childStats; }
		ThreadCpuStats GetChildrenStats() const						{ GenericMutexLock<PosixMutex> l(_mutex); return _childrenStats; }
	};


	STINGRAYKIT_DECLARE_THREAD_LOCAL(ThreadDataStoragePtr, ThreadDataHolder);
	STINGRAYKIT_DEFINE_THREAD_LOCAL(ThreadDataStoragePtr, ThreadDataHolder);


	struct ThreadNameAccessor
	{
	private:
		STINGRAYKIT_DECLARE_THREAD_LOCAL(std::string, ThreadLocalHolder);

	public:
		static const std::string& Get()			{ return ThreadLocalHolder::Get(); }
		static void Set(const std::string& str)
		{
			ThreadLocalHolder::Get() = str;
			ThreadDataStoragePtr& data = ThreadDataHolder::Get();
			if (!data)
				data = make_shared<ThreadDataStorage>(gettid(), pthread_self(), std::string(), null, null);
			data->SetThreadName(str);
		}
	};
	STINGRAYKIT_DEFINE_THREAD_LOCAL(std::string, ThreadNameAccessor::ThreadLocalHolder);


	namespace
	{
		typedef std::map<u64, ThreadDataStoragePtr>	ThreadsMap;

		shared_ptr<PosixMutex>	g_threadsCreatedMutex(new PosixMutex);
		ThreadsMap				g_threadsCreated;


		struct ThreadRegistration
		{
		private:
			shared_ptr<PosixMutex>	_mutex;
			ThreadsMap::iterator	_iterator;

		public:
			ThreadRegistration(const ThreadDataStoragePtr& data) : _mutex(g_threadsCreatedMutex)
			{
				GenericMutexLock<PosixMutex> l(*_mutex);
				_iterator = g_threadsCreated.insert(std::make_pair(data->GetKernelId(), data)).first;
			}

			~ThreadRegistration()
			{
				GenericMutexLock<PosixMutex> l(*_mutex);
				g_threadsCreated.erase(_iterator);
			}
		};


		STINGRAYKIT_DECLARE_PTR(TLSData);
		STINGRAYKIT_DECLARE_THREAD_LOCAL(TLSDataPtr, TLSDataHolder);
		STINGRAYKIT_DEFINE_THREAD_LOCAL(TLSDataPtr, TLSDataHolder);


		TimeDuration TicksToTimeDuration(u64 ticks)
		{ return TimeDuration::FromMilliseconds(ticks * 1000 / sysconf(_SC_CLK_TCK)); }

		ThreadCpuStats CpuStatsFromTicks(u64 uTime, u64 sTime)
		{ return ThreadCpuStats(TicksToTimeDuration(uTime), TicksToTimeDuration(sTime)); }
	}


	class PosixMutexAttr : public Singleton<PosixMutexAttr>
	{
		STINGRAYKIT_SINGLETON(PosixMutexAttr);

	private:
		pthread_mutexattr_t		_mutexAttr;

	private:
		PosixMutexAttr()
		{
			if (int err = pthread_mutexattr_init(&_mutexAttr))
				STINGRAYKIT_THROW(SystemException("pthread_mutexattr_init", err));

			if (int err = pthread_mutexattr_settype(&_mutexAttr, PTHREAD_MUTEX_RECURSIVE))
				STINGRAYKIT_THROW(SystemException("pthread_mutexattr_settype", err));
		}

	public:
		~PosixMutexAttr() { pthread_mutexattr_destroy(&_mutexAttr); }

		const pthread_mutexattr_t& Get() const { return _mutexAttr; }
	};


	PosixMutex::PosixMutex()
	{
		int result = pthread_mutex_init(&_rawMutex, &PosixMutexAttr::Instance().Get());
		if (result != 0)
			STINGRAYKIT_FATAL("pthread_mutex_init");
	}


	void PosixMutex::DoLock(int tryLockResult) const
	{
		if (tryLockResult != EBUSY)
			HandleReturnCode("pthread_mutex_trylock", tryLockResult);

		timespec lock_duration = {};
		lock_duration.tv_sec = 3;
		timespec lock_full_duration = {};

		while (true)
		{
			timespec t = { };
			posix::timespec_now(CLOCK_REALTIME, &t);
			posix::timespec_add(&t, &lock_duration);

			int result = pthread_mutex_timedlock(&_rawMutex, &t);
			if (result == 0)
				return;
			else if (result == ETIMEDOUT)
			{
				posix::timespec_add(&lock_full_duration, &lock_duration);

				TLSData* tls = PosixThreadEngine::GetCurrentThreadData();
				bool is_in_sync_primitive_code = tls ? tls->IsInSyncPrimitiveCode() : false;
				if (tls)
					tls->EnterSyncPrimitiveCode();
				STINGRAYKIT_SCOPE_EXIT(MK_PARAM(TLSData*, tls))
					if (tls)
						tls->LeaveSyncPrimitiveCode();
				STINGRAYKIT_SCOPE_EXIT_END;

				if (!is_in_sync_primitive_code)
				{
					int owner = _rawMutex.__data.__owner; // Cannot use _rawMutex.__data.__lock without libc internal headers, so owner may contain bad values
					std::string owner_name;
					{
						GenericMutexLock<PosixMutex> l(*g_threadsCreatedMutex);
						ThreadsMap::iterator it = g_threadsCreated.find(owner);
						if (it != g_threadsCreated.end())
						{
							owner_name = it->second->GetThreadName();
							IThreadInfoPtr threadInfo = it->second->GetThreadInfo();
							if (threadInfo)
								threadInfo->RequestBacktrace();
						}
					}

					std::string backtrace = Backtrace().Get();
					PTELogger.Warning() << "Could not lock mutex (0x" << Hex(this, 8) << ", owned by "  << (owner_name.empty() ? ToString(owner) : "'" + owner_name + "'")<< ") for "
						<< (lock_full_duration.tv_sec ? ToString(lock_full_duration.tv_sec) + " s " : "")
						<< (lock_full_duration.tv_nsec ? ToString(lock_full_duration.tv_nsec / 1000000) + " ms" : "")
						<< ", there is probably a deadlock" << (backtrace.empty() ? "" : ("\nbacktrace: " + backtrace));
				}
			}
			else
				HandleReturnCode("pthread_mutex_timedlock", result);
		}
	}


	void PosixMutex::HandleReturnCode(const char *where, int code) const
	{
		STINGRAYKIT_THROW(LogicException(std::string(where) + " : " + SystemException::GetErrorMessage(code)));
	}


	class PosixThreadAttr
	{
	public:
		static const size_t DefaultStackSize = 1024 * 1024;

		struct Impl
		{
		private:
			pthread_attr_t _threadAttr;

		public:
			Impl()
			{
				int ret = pthread_attr_init(&_threadAttr);
				STINGRAYKIT_CHECK(ret == 0, SystemException("pthread_attr_init", ret));

				ret = pthread_attr_setstacksize(&_threadAttr, DefaultStackSize);
				STINGRAYKIT_CHECK(ret == 0, SystemException("pthread_attr_setstacksize", ret));
			}

			~Impl()
			{ pthread_attr_destroy(&_threadAttr); }

			const pthread_attr_t& Get() const
			{ return _threadAttr; }
		};
		STINGRAYKIT_DECLARE_PTR(Impl);

	public:
		static ImplPtr Get()
		{
			ImplPtr result = SafeSingleton<Impl>::Instance();
			return result ? result : make_shared<Impl>();
		}
	};


#ifdef HACK_THROW_FROM_PTHREAD_CLEANUP_HANDLER
	struct InterruptException
	{ static void Throw(void*) { throw InterruptException(); } };
#endif


	class PosixThread : public virtual IThread
	{
		STINGRAYKIT_NONCOPYABLE(PosixThread);

	private:
		static const u64 StartingTimeDurationLimit = 1000;

	private:
		function<void (const ICancellationToken&)>	_func;
		std::string									_initialName;
		ThreadDataStoragePtr						_parent;

		const std::string							_name;

		PosixMutex									_mutex;
		PosixConditionVariable						_cv;
		bool										_started;

		ThreadDataStoragePtr						_data;

		bool										_exited;
		CancellationToken							_token;
		TaskLifeToken								_lifeToken;

	public:
		PosixThread(const function<void (const ICancellationToken&)>& func, const std::string& name, const ThreadDataStoragePtr& parent) :
			_func(func), _initialName(name), _parent(parent), _started(false), _exited(false)
		{
			for (int attempt = 0; attempt < 3; ++attempt)
			{
				pthread_t id;
				int ret = pthread_create(&id, &PosixThreadAttr::Get()->Get(), &PosixThread::ThreadFuncStatic, this);
				if (ret == 0)
				{
					{
						GenericMutexLock<PosixMutex> l(_mutex);
						while (!_started)
							_cv.Wait(_mutex, DummyCancellationToken());
					}

					STINGRAYKIT_CHECK(_data, "PosixThread failed to start!");
					STINGRAYKIT_ASSERT(id == _data->GetPthreadId());
					return;
				}

				if (ret == EAGAIN)
				{
					PTELogger.Warning() << "pthread_create failed with EAGAIN, retrying...";
					Thread::Sleep(100);
				}
				else
					STINGRAYKIT_FATAL("pthread_create: ret = " + ToString(ret) + ", " + SystemException::GetErrorMessage(ret));
			}
			STINGRAYKIT_FATAL("pthread_create failed too much times!");
		}


		virtual ~PosixThread()
		{
			AsyncProfiler::Session profile_session(ExecutorsProfiler::Instance().GetProfiler(), bind(&PosixThread::GetAsyncProfilerMessage, this), ThreadDtorProfileReportThreshold, AsyncProfiler::Session::Behaviour::Silent, AsyncProfiler::Session::NameGetterTag());

			_token.Cancel();
			_lifeToken.Release();

			bool prev = PosixThreadEngine::EnableInterruptionPoints(false);
			ScopeExitInvoker sei(bind(&PosixThreadEngine::EnableInterruptionPoints, prev));

			int result = pthread_join(_data->GetPthreadId(), NULL);
			if (result)
				STINGRAYKIT_FATAL("pthread_join failed: " + SystemException::GetErrorMessage(result));

			if (!_exited)
				PTELogger.Error() << "Critical: Alert! Threadfunc never exited!";
		}


		virtual void Interrupt()
		{
			PTELogger.Warning() << "Interrupting thread " << _name << ", backtrace: " << Backtrace();
			RequestThreadBacktrace();
#ifdef PLATFORM_NP6
#	warning Thread interruptions disabled
#else
			int ret = pthread_cancel(_data->GetPthreadId());
			if (ret != 0)
			{
				if (ret == ESRCH)
				{
					Logger::Info() << "no thread with the ID thread (" << _data->GetPthreadId() << ") could be found (threadfunc exited)";
					return;
				}

				STINGRAYKIT_FATAL("pthread_cancel: " + SystemException::GetErrorMessage(ret));
			}
#endif
		}

	private:
		void RequestThreadBacktrace() const
		{ posix::SendSignal(_data->GetPthreadId(), g_threadBacktracePrinter.GetSignalNum()); }


		std::string GetAsyncProfilerMessage() const
		{
			RequestThreadBacktrace();
			return _name + ": destroying";
		}


		void ThreadFuncExited()
		{
			_exited = true;

			ThreadCpuStats cpuStats;
			Stat s;
			if (Stat::GetThreadStats(getpgid((pid_t)0), GetKernelId(), s))
				cpuStats = CpuStatsFromTicks(s.utime, s.stimev);
			else
				PTELogger.Debug() << "Can't get thread stats for thread '" << _data->GetThreadName() << "' with tid = " << GetKernelId();

			ThreadCpuStats childrenStats = _data->GetChildrenStats();
			PTELogger.Debug() << "Exiting threadfunc of thread '" << _data->GetThreadName() << "' with tid = " << GetKernelId() << ". Stats: " << cpuStats << ". Children stats: " << childrenStats;

			ThreadDataStoragePtr parent = _data->GetParentThread();
			if (parent)
				parent->StoreChildStats(cpuStats + childrenStats);
		}


		static void* ThreadFuncStatic(void* instancePtr)
		{
			PosixThread* t = static_cast<PosixThread*>(instancePtr);
			t->ThreadFunc();
			return NULL;
		}


		u64 GetKernelId()
		{ return gettid(); }


		void ThreadFuncStarted()
		{
			GenericMutexLock<PosixMutex> l(_mutex);
			_started = true;
			_cv.Broadcast();
		}


		void ThreadFunc()
		{
			ScopeExitInvoker sei(bind(&PosixThread::ThreadFuncStarted, this)); // in case something throws an exception

			int old_state = 0, old_type = 0;
			if (pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &old_type) != 0)
				return;
			if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &old_state) != 0)
				return;

#ifdef HAVE_PTHREAD_SETNAME
			pthread_setname_np(pthread_self(), _initialName.c_str());
#endif

			// PR_SET_NAME requires string of 16 bytes maximum
			std::string cropped_name(_initialName.substr(0, 16));
			prctl(PR_SET_NAME, cropped_name.c_str());

#ifdef HACK_THROW_FROM_PTHREAD_CLEANUP_HANDLER
			pthread_cleanup_push(&InterruptException::Throw, NULL);
#endif

			_data = make_shared<ThreadDataStorage>(gettid(), pthread_self(), _initialName, _parent, _lifeToken.GetExecutionTester());
			ThreadDataHolder::Get() = _data;
			ThreadNameAccessor::Set(_initialName);
			ThreadFuncStarted();

			ThreadRegistration registrator(_data);

			TLSDataPtr tlsData = make_shared<TLSData>();
			TLSDataHolder::Get() = tlsData;

			try
			{
				PTELogger.Debug() << "Entered threadfunc of thread '" << _data->GetThreadName() << "' with tid = " << GetKernelId();
				ScopeExitInvoker sei(bind(&PosixThread::ThreadFuncExited, this));

				_func(_token); // Execute!
			}
			catch (const std::exception& ex)
			{
				PTELogger.Error() << "Uncaught std::exception:" << ex << "\n   in thread with tid = " << GetKernelId();
			}
#ifdef HACK_THROW_FROM_PTHREAD_CLEANUP_HANDLER
			catch (const InterruptException&)
			{
				PTELogger.Info() << "Thread with id = " << GetKernelId() << " interrupted";
			}
#endif
#ifdef HAVE_ABI_FORCE_UNWIND
			catch (abi::__forced_unwind&)
			{
				PTELogger.Debug() << "Thread with id = " << GetKernelId() << " interrupted";
				throw;
			}
			catch (...)
			{
				PTELogger.Error() << "Uncaught unknown exception\n    in thread with tid = " << GetKernelId();
				throw;
			}
#else
			catch (...)
			{
				PTELogger.Debug() << "Uncaught unknown exception\n    in thread with tid = " << GetKernelId();
				throw;
			}
#endif

#ifdef HACK_THROW_FROM_PTHREAD_CLEANUP_HANDLER
			pthread_cleanup_pop(0);
#endif
		}
	};
	STINGRAYKIT_DECLARE_PTR(PosixThread);


	IThreadPtr PosixThreadEngine::BeginThread(const FuncType& func, const std::string& name)
	{
		ThreadDataStoragePtr parent;
		{
			GenericMutexLock<PosixMutex> l(*g_threadsCreatedMutex);
			ThreadsMap::iterator it = g_threadsCreated.find(gettid());
			if (it != g_threadsCreated.end())
				parent = it->second;
		}

		return make_shared<PosixThread>(func, name, parent);
	}


	void PosixThreadEngine::Yield()
	{
#if HAVE_PTHREAD_YIELD
		if (pthread_yield() != 0)
			PTELogger.Warning() << "pthread_yield failed!";
#else
		STINGRAYKIT_TRY("[PosixThreadEngine] Sleep failed", Sleep(0));
#endif
	}


	void PosixThreadEngine::SleepMicroseconds(u64 microseconds)
	{
		timespec req = { };
		timespec rem = { };

		req.tv_sec = microseconds / 1000000;
		req.tv_nsec = (microseconds % 1000000) * 1000;

		while (nanosleep(&req, &rem) != 0)
		{
			if (errno != EINTR)
				STINGRAYKIT_THROW(SystemException("nanosleep"));

			req = rem;
		}
	}


	void PosixThreadEngine::InterruptionPoint()
	{
		pthread_testcancel();
	}


	bool PosixThreadEngine::EnableInterruptionPoints(bool enable)
	{
		int state = enable ? PTHREAD_CANCEL_ENABLE : PTHREAD_CANCEL_DISABLE, old_state = 0;
		if (int err = pthread_setcancelstate(state, &old_state))
			STINGRAYKIT_THROW(SystemException("pthread_setcancelstate", err));

		return old_state == PTHREAD_CANCEL_ENABLE;
	}


	IThread::ThreadId PosixThreadEngine::GetCurrentThreadId()
	{ return gettid(); }


	IThreadInfoPtr PosixThreadEngine::GetCurrentThreadInfo()
	{
		ThreadDataStoragePtr dataStorage = ThreadDataHolder::Get();
		return dataStorage ? dataStorage->GetThreadInfo() : null;
	}


	TLSData* PosixThreadEngine::GetCurrentThreadData()
	{
		if (!TLSDataHolder::Get())
			TLSDataHolder::Get() = make_shared<TLSData>();
		return TLSDataHolder::Get().get();
	}


	void PosixThreadEngine::SetCurrentThreadName(const std::string& name)
	{ ThreadNameAccessor::Set(name); }


	const std::string& PosixThreadEngine::GetCurrentThreadName()
	{ return ThreadNameAccessor::Get(); }


	PosixThreadEngine::ThreadStatsVec PosixThreadEngine::GetThreadsStats()
	{
		GenericMutexLock<PosixMutex> l(*g_threadsCreatedMutex);

		ThreadStatsVec result;

		pid_t gid = getpgid((pid_t)0);
		STINGRAYKIT_CHECK(gid != -1, SystemException("getpgid"));

		ScopedHolder<DIR*> rootDir(opendir(("/proc/" + ToString(gid) + "/task").c_str()), &closedir);
		STINGRAYKIT_CHECK(rootDir.Valid(), SystemException("opendir"));

		struct dirent* dirEntry;
		while ((dirEntry = readdir(rootDir.Get())) != NULL)
		{
			std::string dirName(dirEntry->d_name);
			if (dirName == "." || dirName == "..")
				continue;

			struct stat st;
			if (fstatat(dirfd(rootDir.Get()), dirName.c_str(), &st, 0) != 0)
				continue;

			Stat stat;
			u64 tid = FromString<u64>(dirName);
			if (S_ISDIR(st.st_mode) && Stat::GetThreadStats(gid, tid, stat))
			{
				std::string threadName(stat.tcomm);
				ThreadCpuStats childrenStats;
				u64 parentId = 0; //stat.ppid;
				if (g_threadsCreated.find(tid) != g_threadsCreated.end())
				{
					threadName = g_threadsCreated[tid]->GetThreadName();
					childrenStats = g_threadsCreated[tid]->GetChildrenStats();
					if (g_threadsCreated[tid]->GetParentThread())
						parentId = g_threadsCreated[tid]->GetParentThread()->GetKernelId();
				}

				result.push_back(ThreadStats(tid, parentId, threadName, CpuStatsFromTicks(stat.utime, stat.stimev), childrenStats));
			}
		}

		return result;
	}


	struct SchedulingPolicyMapper : public BaseValueMapper<SchedulingPolicyMapper, ThreadSchedulingPolicy::Enum, int>
	{
		typedef TypeList_6<
			Src::Value<ThreadSchedulingPolicy::NonRealtime>,		Dst::Value<SCHED_OTHER>,
			Src::Value<ThreadSchedulingPolicy::RealtimeRoundRobin>,	Dst::Value<SCHED_RR>,
			Src::Value<ThreadSchedulingPolicy::RealtimeFIFO>,		Dst::Value<SCHED_FIFO> > MappingsList;

		typedef TypeList_2<Src::Fail, Dst::Fail > DefaultMapping;
	};


	ThreadSchedulingParams PosixThreadEngine::SetCurrentThreadPriority(ThreadSchedulingParams params)
	{
		int old_policy = SCHED_OTHER;
		struct sched_param old_scheduler_params = {};
		int res = pthread_getschedparam(pthread_self(), &old_policy, &old_scheduler_params);
		STINGRAYKIT_CHECK(res == 0, SystemException("pthread_getschedparam", res));

		int policy = SchedulingPolicyMapper::Map(params.GetPolicy());
		int min_priority = sched_get_priority_min(policy);
		int max_priority = sched_get_priority_max(policy);
		struct sched_param scheduler_params = {};
		scheduler_params.sched_priority = std::min(std::max(params.GetPriority(), min_priority), max_priority);

		res = pthread_setschedparam(pthread_self(), policy, &scheduler_params);
		STINGRAYKIT_CHECK(res == 0, SystemException("pthread_setschedparam", res));

		return ThreadSchedulingParams(SchedulingPolicyMapper::Unmap(old_policy), old_scheduler_params.sched_priority);
	}

}
