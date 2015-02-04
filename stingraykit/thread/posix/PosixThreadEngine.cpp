#include <stingraykit/thread/posix/PosixThreadEngine.h>

#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>


#ifdef HAVE_ABI_FORCE_UNWIND
#include <cxxabi.h>
#endif

#include <memory>

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
#include <stingraykit/thread/GenericMutexLock.h>
#include <stingraykit/thread/posix/SignalHandler.h>
#include <stingraykit/time/posix/utils.h>


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


	class ThreadDataStorage;
	STINGRAYKIT_DECLARE_PTR(ThreadDataStorage);


	class PosixThreadInfo : public virtual IThreadInfo
	{
	private:
		pthread_t					_threadHandle;
		std::string					_name;
		FutureExecutionTester		_threadGuard;

	public:
		PosixThreadInfo(const pthread_t& threadHandle, const std::string& name, const FutureExecutionTester& threadGuard)
			: _threadHandle(threadHandle), _name(name), _threadGuard(threadGuard)
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



	class ThreadDataStorage
	{
		std::string				_name;
		optional<u64>			_tid;
		optional<pthread_t>		_threadHandle;
		PosixThreadInfoPtr		_threadInfo;
		ThreadDataStoragePtr	_parent;

		atomic_int_type			_exitedChildrenUTime;
		atomic_int_type			_exitedChildrenSTime;

		atomic_int_type			_threadFuncExited;

		CancellationToken		_token;
		TaskLifeToken			_lifeToken;

	public:
		ThreadDataStorage(const std::string& name, const ThreadDataStoragePtr& parent) :
			_name(name),
			_parent(parent),
			_exitedChildrenUTime(0),
			_exitedChildrenSTime(0),
			_threadFuncExited(0)
		{ }

		~ThreadDataStorage()
		{ }

		void SetTid(u64 tid) 							{ _tid = tid; }
		u64 GetTid() const								{ return _tid.get(); }

		void SetThreadHandle(const pthread_t& handle)
		{
			_threadHandle = handle;
			_threadInfo.reset(new PosixThreadInfo(handle, _name, _lifeToken.GetExecutionTester()));
		}
		pthread_t GetThreadHandler() const				{ return *_threadHandle; }

		PosixThreadInfoPtr GetThreadInfo() const		{ return _threadInfo; }

		const std::string& GetThreadName() const		{ return _name; }
		ThreadDataStoragePtr GetParentThread() const	{ return _parent; }

		void StoreChildTime(u64 utime, u64 stime)
		{
			Atomic::Add(_exitedChildrenUTime, utime);
			Atomic::Add(_exitedChildrenSTime, stime);
		}

		u64 GetChildrenUTime()					{ return Atomic::Load(_exitedChildrenUTime); }
		u64 GetChildrenSTime()					{ return Atomic::Load(_exitedChildrenSTime); }

		void SetExited()						{ Atomic::Store(_threadFuncExited, 1); }
		bool IsExited()							{ return Atomic::Load(_threadFuncExited) == 1; }

		CancellationToken& GetToken()			{ return _token; }
		TaskLifeToken& GetLifeToken()			{ return _lifeToken; }
	};


	namespace
	{

		typedef std::map<u64, ThreadDataStoragePtr>	ThreadsMap;

		shared_ptr<PosixMutex>	g_threadsCreatedMutex(new PosixMutex);
		ThreadsMap				g_threadsCreated;


		struct ThreadRegistration
		{
			u64						Id;
			ThreadDataStoragePtr	Data;
		private:
			shared_ptr<PosixMutex> _mutex;

		public:
			ThreadRegistration(u64 id, const pthread_t& threadHandle, const ThreadDataStoragePtr& data)
				: Id(id), Data(data), _mutex(g_threadsCreatedMutex)
			{
				Data->SetTid(id);
				Data->SetThreadHandle(threadHandle);
				GenericMutexLock<PosixMutex> l(*_mutex);
				g_threadsCreated.insert(std::make_pair(Id, Data));
			}

			~ThreadRegistration()
			{
				GenericMutexLock<PosixMutex> l(*_mutex);
				g_threadsCreated.erase(Id);
			}
		};


		struct TLSDataHolder
		{
			TLSData					tlsData;
			ThreadRegistration		threadReg;

			TLSDataHolder(u64 id, const pthread_t& threadHandle, const TLSData& theTlsData, const ThreadDataStoragePtr& threadData)
				: tlsData(theTlsData), threadReg(id, threadHandle, threadData)
			{ }

			STINGRAYKIT_NONCOPYABLE(TLSDataHolder);
		};


		static pthread_once_t	g_TLS_keyOnce = PTHREAD_ONCE_INIT;


		class TLS
		{
		private:
			static pthread_key_t	s_key;

		public:
			static void Init(const TLSData& tlsData, const ThreadDataStoragePtr& threadData)
			{
				if (Get() != NULL)
					STINGRAYKIT_THROW(std::runtime_error("TLS data already assigned!"));

				SetValue(tlsData, threadData);
			}

			template < typename ThreadDataStorageCreator >
			static void TryInit(const TLSData& tlsData, const ThreadDataStorageCreator& threadDataCreator)
			{
				if (Get() != NULL)
					return;

				SetValue(tlsData, threadDataCreator());
			}

			static TLSData* Get()
			{
				InitKey();
				TLSDataHolder* holder = reinterpret_cast<TLSDataHolder*>(pthread_getspecific(s_key));
				return holder ? &holder->tlsData : NULL;
			}

			static ThreadDataStorage* GetPrivateData()
			{
				InitKey();
				TLSDataHolder* holder = reinterpret_cast<TLSDataHolder*>(pthread_getspecific(s_key));
				return holder ? holder->threadReg.Data.get() : NULL;
			}

		private:
			static void InitKey()
			{
				if (pthread_once(&g_TLS_keyOnce, &TLS::DoInit) != 0)
					STINGRAYKIT_THROW(SystemException("pthread_once"));
			}

			static void SetValue(const TLSData& tlsData, const ThreadDataStoragePtr& threadData)
			{
				std::auto_ptr<TLSDataHolder> tls_ptr(new TLSDataHolder(gettid(), pthread_self(), tlsData, threadData));
				if (pthread_setspecific(s_key, tls_ptr.get()) != 0)
					STINGRAYKIT_THROW(SystemException("pthread_setspecific"));
				tls_ptr.release();
			}

			static void DoInit()
			{
				if (pthread_key_create(&s_key, &TLS::Dtor) != 0)
					STINGRAYKIT_THROW(SystemException("pthread_key_create"));
			}

			static void Dtor(void* val)
			{
				TLSDataHolder* holder = reinterpret_cast<TLSDataHolder*>(val);
				delete holder;
			}
		};


		pthread_key_t	TLS::s_key;


		inline u64 TicksToMs(u64 ticks)
		{
			return ticks * 1000 / sysconf(_SC_CLK_TCK);
		}

	}


	class PosixMutexAttr : public Singleton<PosixMutexAttr>
	{
		STINGRAYKIT_SINGLETON(PosixMutexAttr);

	private:
		pthread_mutexattr_t		_mutexAttr;

	private:
		PosixMutexAttr()
		{
			if (pthread_mutexattr_init(&_mutexAttr) != 0)
				STINGRAYKIT_THROW(SystemException("pthread_mutexattr_init"));

			if (pthread_mutexattr_settype(&_mutexAttr, PTHREAD_MUTEX_RECURSIVE) != 0)
				STINGRAYKIT_THROW(SystemException("pthread_mutexattr_settype"));
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


	void PosixMutex::Lock() const
	{
		int result = pthread_mutex_trylock(&_rawMutex);
		if (result == 0)
			return;
		else if (result != EBUSY)
			HandleReturnCode("pthread_mutex_trylock", result);

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


	void PosixMutex::Unlock() const
	{
		int result = pthread_mutex_unlock(&_rawMutex);
		if (result != 0)
			HandleReturnCode("pthread_mutex_unlock", result);
	}


	void PosixMutex::HandleReturnCode(const char *where, int code) const
	{
		STINGRAYKIT_THROW(LogicException(std::string(where) + " : " + SystemException::GetErrorMessage(code)));
	}


	namespace
	{
		struct Stat
		{
			// 0
			s64				pid;
			char			tcomm[1024];
			char			state;
			s64				ppid;
			s64				pgid;
			// 5
			s64				sid;
			s64				tty_nr;
			s64				tty_pgrp;
			s64				flags;
			s64				min_flt;
			// 10
			s64				cmin_flt;
			s64				maj_flt;
			s64				cmaj_flt;
			s64				utime;
			s64				stimev;
			// 15
			s64				cutime;
			s64				cstime;
			s64				priority;
			s64				nicev;
			s64				num_threads;
			// 20
			s64				it_real_value;
			u64				start_time;
			s64				vsize;
			s64				rss;
			s64				rsslim;
			// 25
			s64				start_code;
			s64				end_code;
			s64				start_stack;
			s64				esp;
			s64				eip;
			// 30
			s64				pending;
			s64				blocked;
			s64				sigign;
			s64				sigcatch;
			s64				wchan;
			// 35
			s64				zero1;
			s64				zero2;
			s64				exit_signal;
			s64				processor;
			s64				rt_priority;
			// 40
			s64				policy;

			std::string ToString() const
			{
				string_ostream ss;
				ss		<< "pid: " << pid << ", tcomm: " << std::string(tcomm) << ", state: " << state << ", ppid: " << ppid << ", pgid: " << pgid
						<< ", sid: " << sid << ", tty_nr: " << tty_nr << ", tty_pgrp: " << tty_pgrp << ", flags: " << flags << ", min_flt: " << min_flt
						<< ", cmin_flt: " << cmin_flt << ", maj_flt: " << maj_flt << ", cmin_flt: " << cmaj_flt << ", utime: " << utime << ", stimev: " << stimev
						<< ", cutime: " << cutime << ", cstime: " << cstime << ", priority: " << priority << ", nicev: " << nicev << ", num_threads: " << num_threads
						<< ", it_real_value: " << it_real_value << ", start_time: " << start_time << ", vsize: " << vsize << ", rss: " << rss << ", rsslim: " << rsslim
						<< ", start_code: " << start_code << ", end_code: " << end_code << ", start_stack: " << start_stack << ", esp: " << esp << ", eip: " << eip
						<< ", pending: " << pending << ", blocked: " << blocked << ", sigign: " << sigign << ", sigcatch: " << sigcatch << ", wchan: " << wchan
						<< ", zero1: " << zero1 << ", zero2: " << zero2 << ", exit_signal: " << exit_signal << ", processor: " << processor << ", rt_priority: " << rt_priority
						<< ", policy: " << policy
				;
				return ss.str();
			}
		};
	}


	static void ReadCpuTimeFromStat(FILE* stat, Stat& s)
	{
		int count = fscanf(stat, "%lld (%1024[^)]) %c %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %llu %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld",
		// 0
		&s.pid,
		s.tcomm,
		&s.state,
		&s.ppid,
		&s.pgid,
		// 5
		&s.sid,
		&s.tty_nr,
		&s.tty_pgrp,
		&s.flags,
		&s.min_flt,
		// 10
		&s.cmin_flt,
		&s.maj_flt,
		&s.cmaj_flt,
		&s.utime,
		&s.stimev,
		// 15
		&s.cutime,
		&s.cstime,
		&s.priority,
		&s.nicev,
		&s.num_threads,
		// 20
		&s.it_real_value,
		&s.start_time,
		&s.vsize,
		&s.rss,
		&s.rsslim,
		// 25
		&s.start_code,
		&s.end_code,
		&s.start_stack,
		&s.esp,
		&s.eip,
		// 30
		&s.pending,
		&s.blocked,
		&s.sigign,
		&s.sigcatch,
		&s.wchan,
		// 35
		&s.zero1,
		&s.zero2,
		&s.exit_signal,
		&s.processor,
		&s.rt_priority,
		// 40
		&s.policy);

		static const int ArgsCount = 41;
		if (count != ArgsCount)
			PTELogger.Warning() << "Can't read CPU time from stat!";
	}


	static bool GetThreadStats(u64 gid, u64 id, Stat& stat)
	{
#ifndef PLATFORM_STAPI
		std::string filename = "/proc/" + ToString(gid) + "/task/" + ToString(id) + "/stat";
#else
		// Looks like in ST linux correct CPU usage for thread 123 is accessible at /proc/123/task/123/stat
		std::string filename = "/proc/" + ToString(id) + "/task/" + ToString(id) + "/stat";
#endif

		FILE* stat_f = fopen(filename.c_str(), "r");
		if (stat_f)
		{
			ScopeExitInvoker sei(bind(&fclose, stat_f));
			ReadCpuTimeFromStat(stat_f, stat);
		}
		else
		{
			filename = "/proc/" + ToString(id) + "/stat";
			stat_f = fopen(filename.c_str(), "r");
			if (!stat_f)
				return false;

			ScopeExitInvoker sei(bind(&fclose, stat_f));
			ReadCpuTimeFromStat(stat_f, stat);
		}
		return true;
	}


	class PosixThread : public virtual IThread
	{
		STINGRAYKIT_NONCOPYABLE(PosixThread);

	private:
		ThreadDataStoragePtr	_data;

		const std::string		_name;

		Mutex					_mutex;
		optional<pthread_t>		_id;

	public:
		PosixThread(const ThreadDataStoragePtr& data, const std::string& name) :
			_data(data),
			_name(name)
		{ }

		virtual ~PosixThread()
		{
			AsyncProfiler::Session profile_session(ExecutorsProfiler::Instance().GetProfiler(), bind(&PosixThread::GetAsyncProfilerMessage, this), ThreadDtorProfileReportThreshold, AsyncProfiler::Session::Behaviour::Silent, AsyncProfiler::Session::NameGetterTag());
			_data->GetToken().Cancel();
			_data->GetLifeToken().Release();
			Join();
		}

		void SetThreadId(const pthread_t& id)
		{ GenericMutexLock<PosixMutex> l(_mutex); _id = id; }

		pthread_t GetThreadId()
		{ GenericMutexLock<PosixMutex> l(_mutex); return *_id; }

		void Join()
		{
			TRACER;
			bool prev = PosixThreadEngine::EnableInterruptionPoints(false);
			ScopeExitInvoker sei(bind(&PosixThreadEngine::EnableInterruptionPoints, prev));

			int result = pthread_join(GetThreadId(), NULL);
			if (result)
			{
				std::string backtrace = Backtrace().Get();
				STINGRAYKIT_FATAL("pthread_join failed: " + SystemException::GetErrorMessage(result) + (backtrace.empty() ? "" : ("\nbacktrace: " + backtrace)));
			}

			if (!_data->IsExited())
				PTELogger.Error() << "Critical: Alert! Threadfunc never exited!";
		}

		virtual void Interrupt()
		{
			PTELogger.Warning() << "Interrupting thread " << _name << ", backtrace: " << Backtrace();
			RequestThreadBacktrace();
#ifdef PLATFORM_NP6
#	warning Thread interruptions disabled
#else
			//this flag could be unreliable, consider remove it or made atomic with save/load
			if (_data->IsExited())
				return;

			int ret = pthread_cancel(GetThreadId());
			if (ret != 0)
			{
				if (ret == ESRCH)
				{
					Logger::Info() << "no thread with the ID thread (" + ToString(GetThreadId()) + ") could be found (threadfunc exited)";
					return;
				}

				STINGRAYKIT_FATAL("pthread_cancel: " + SystemException::GetErrorMessage(ret));
			}
#endif
		}

	private:
		void RequestThreadBacktrace() const
		{
			GenericMutexLock<PosixMutex> l(_mutex);
			if (_id)
				posix::SendSignal(*_id, g_threadBacktracePrinter.GetSignalNum());
		}

		std::string GetAsyncProfilerMessage() const
		{
			RequestThreadBacktrace();
			return _name + ": destroying";
		}

	};
	STINGRAYKIT_DECLARE_PTR(PosixThread);


	class PosixThreadAttr
	{
	public:
		static const size_t DefaultStackSize = 1024 * 1024;

	private:
		pthread_attr_t _threadAttr;

	private:
		PosixThreadAttr()
		{
			int ret = pthread_attr_init(&_threadAttr);
			if (ret)
				STINGRAYKIT_THROW(SystemException("pthread_attr_init", ret));

			SetStackSize(DefaultStackSize);
		}

	public:
		~PosixThreadAttr()
		{ pthread_attr_destroy(&_threadAttr); }

		const pthread_attr_t& Get() const { return _threadAttr; }

		size_t GetStackSize() const
		{
			size_t size;
			int ret = pthread_attr_getstacksize(&_threadAttr, &size);
			if (ret)
				STINGRAYKIT_THROW(SystemException("pthread_attr_getstacksize", ret));
			return size;
		}

		void SetStackSize(size_t size)
		{
			int ret = pthread_attr_setstacksize(&_threadAttr, size);
			if (ret)
				STINGRAYKIT_THROW(SystemException("pthread_attr_setstacksize", ret));
		}

		static const PosixThreadAttr& Instance() // Do not want to use Singleton class here 'cause it may depend on these threading objects
		{
			static PosixThreadAttr inst;
			return inst;
		}
	};


	class ThreadArg
	{
	private:
		typedef function<void(const ICancellationToken& token)> FuncType;

		FuncType				_func;
		TLSData					_tlsData;
		ThreadDataStoragePtr	_threadData;

	public:
		ThreadArg(const FuncType& func, const TLSData& tlsData, const ThreadDataStoragePtr& threadData)
			: _func(func), _tlsData(tlsData), _threadData(threadData)
		{ }

		const FuncType& GetFunc() const						{ return _func; }
		const TLSData& GetTLSData() const					{ return _tlsData; }
		const ThreadDataStoragePtr& GetThreadData() const	{ return _threadData; }
	};


#ifdef HACK_THROW_FROM_PTHREAD_CLEANUP_HANDLER
	struct InterruptException
	{ static void Throw(void*) { throw InterruptException(); } };
#endif


	void* PosixThreadEngine::ThreadFunc(void* arg)
	{
		class ThreadDetacher
		{
			ThreadDataStoragePtr	_data;

		public:
			ThreadDetacher(const ThreadDataStoragePtr& data) : _data(data)
			{ }

			~ThreadDetacher()
			{ _data->SetExited(); }
		};

		int old_state = 0, old_type = 0;
		if (pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &old_type) != 0)
			return (void*)-1;
		if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &old_state) != 0)
			return (void*)-1;

		std::auto_ptr<ThreadArg> thread_arg(reinterpret_cast<ThreadArg*>(arg));

		class ThreadFuncLogger
		{
		private:
			u64						_tid;
			ThreadDataStoragePtr	_data;
			bool					_testMode;

		public:
			ThreadFuncLogger(const ThreadDataStoragePtr& data) : _tid(gettid()), _data(data), _testMode(getenv("TEST_MODE") != NULL)
			{}

			void LogEnter()
			{
				if (!_testMode)
					PTELogger.Debug() << "Entered threadfunc of thread '" << _data->GetThreadName() << "' with tid = " << _tid;
			}

			~ThreadFuncLogger()
			{
				u64 utime = 0, stime = 0;

				Stat s;
				if (GetThreadStats(getpgid((pid_t)0), _tid, s))
				{
					utime = s.utime;
					stime = s.stimev;
				}

				std::string stat = StringFormat("u: %1% ms, s: %2% ms", TicksToMs(utime), TicksToMs(stime));
				if (_data->GetChildrenUTime() != 0 || _data->GetChildrenSTime() != 0)
				{
					std::string childrenStat = StringFormat("u: %1% ms, s: %2% ms", TicksToMs(_data->GetChildrenUTime()), TicksToMs(_data->GetChildrenSTime()));
					if (!_testMode)
						PTELogger.Debug() << "Exiting threadfunc of thread '" << _data->GetThreadName() << "' with tid = " << _tid << ". Stats: " << stat << ". Children stats: " << childrenStat;
				}
				else
				{
					if (!_testMode)
						PTELogger.Debug() << "Exiting threadfunc of thread '" << _data->GetThreadName() << "' with tid = " << _tid << ". Stats: " << stat;
				}

				ThreadDataStoragePtr parent = _data->GetParentThread();
				if (parent)
					parent->StoreChildTime(utime + _data->GetChildrenUTime(), stime + _data->GetChildrenSTime());
			}

			size_t GetId() const { return _tid; }
		};

		ThreadFuncLogger tfl(thread_arg->GetThreadData());

		// PR_SET_NAME requires string of 16 bytes maximum
		std::string cropped_name(thread_arg->GetTLSData().GetThreadName().substr(0, 16));
		prctl(PR_SET_NAME, cropped_name.c_str());

		int ret = 0;

#ifdef HACK_THROW_FROM_PTHREAD_CLEANUP_HANDLER
		pthread_cleanup_push(&InterruptException::Throw, NULL);
#endif

		try
		{
			ThreadDetacher detacher(thread_arg->GetThreadData());

			TLS::Init(thread_arg->GetTLSData(), thread_arg->GetThreadData());

			tfl.LogEnter();

			(thread_arg->GetFunc())(thread_arg->GetThreadData()->GetToken()); // Execute!
		}
		catch (const std::exception& ex)
		{
			PTELogger.Error() << "Uncaught std::exception:" << ex << "\n   in thread with tid = " << tfl.GetId();
			ret = 1;
		}
#ifdef HACK_THROW_FROM_PTHREAD_CLEANUP_HANDLER
		catch (const InterruptException&)
		{
			PTELogger.Info() << "Thread with id = " << tfl.GetId() << " interrupted";
			ret = 1;
		}
#endif
#ifdef HAVE_ABI_FORCE_UNWIND
		catch (abi::__forced_unwind&)
		{
			PTELogger.Debug() << "Thread with id = " << tfl.GetId() << " interrupted";
			throw;
		}
		catch (...)
		{
			PTELogger.Error() << "Uncaught unknown exception\n    in thread with tid = " << tfl.GetId();
			throw;
		}
#else
		catch (...)
		{
			PTELogger.Debug() << "Uncaught unknown exception\n    in thread with tid = " << tfl.GetId();
			throw;
		}
#endif

#ifdef HACK_THROW_FROM_PTHREAD_CLEANUP_HANDLER
		pthread_cleanup_pop(0);
#endif

		return reinterpret_cast<void*>(ret);
	}


	IThreadPtr PosixThreadEngine::BeginThread(const FuncType& func, const std::string& name)
	{
		ThreadDataStoragePtr parent_data;
		{
			GenericMutexLock<PosixMutex> l(*g_threadsCreatedMutex);
			ThreadsMap::iterator it = g_threadsCreated.find(gettid());
			if (it != g_threadsCreated.end())
				parent_data = it->second;
		}

		ThreadDataStoragePtr thread_data(new ThreadDataStorage(name, parent_data));
		shared_ptr<PosixThread> thread_obj(new PosixThread(thread_data, name));
		TLSData tls_data(name);
		std::auto_ptr<ThreadArg> thread_arg(new ThreadArg(func, tls_data, thread_data));

		pthread_t pthread_id;
		int ret, attempt = 0;
		do
		{
			ret = pthread_create(&pthread_id, &PosixThreadAttr::Instance().Get(), &PosixThreadEngine::ThreadFunc, thread_arg.get());
			if (ret == EAGAIN)
			{
				PTELogger.Warning() << "pthread_create failed with EAGAIN, retrying...";
				Thread::Sleep(100);
			}
		}
		while(ret == EAGAIN && attempt++ < 3);

		thread_obj->SetThreadId(pthread_id);
#ifdef HAVE_PTHREAD_SETNAME
		pthread_setname_np(pthread_id, tls_data.GetThreadName().c_str());
#endif

		if (ret != 0)
			STINGRAYKIT_FATAL("pthread_create: ret = " + ToString(ret) + ", " + SystemException::GetErrorMessage(ret));
		else
			thread_arg.release();

		return thread_obj;
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
		if (pthread_setcancelstate(state, &old_state) != 0)
			STINGRAYKIT_THROW(SystemException("pthread_setcancelstate"));

		return old_state == PTHREAD_CANCEL_ENABLE;
	}


	IThread::ThreadId PosixThreadEngine::GetCurrentThreadId()
	{ return gettid(); }

	IThreadInfoPtr PosixThreadEngine::GetCurrentThreadInfo()
	{
		ThreadDataStorage* privateData = TLS::GetPrivateData();
		return privateData ? privateData->GetThreadInfo() : null;
	}

	TLSData* PosixThreadEngine::GetCurrentThreadData()
	{ return TLS::Get(); }


	void PosixThreadEngine::SetCurrentThreadName(const std::string& name)
	{
		STINGRAYKIT_TRY("[PosixThreadEngine] failed setting current thread name", TLS::Init(TLSData(name), make_shared<ThreadDataStorage>(name, ThreadDataStoragePtr())));
	}


	static ThreadDataStoragePtr CreateThreadDataStorage(const std::string& name)
	{ return make_shared<ThreadDataStorage>(name, ThreadDataStoragePtr()); }

	void PosixThreadEngine::TrySetCurrentThreadName(const std::string& name)
	{ TLS::TryInit(TLSData(name), bind(&CreateThreadDataStorage, const_ref(name))); }


	PosixThreadEngine::ThreadStatsVec PosixThreadEngine::GetStingrayThreadsStats()
	{
		GenericMutexLock<PosixMutex> l(*g_threadsCreatedMutex);

		ThreadStatsVec result;

		pid_t gid = getpgid((pid_t)0);
		STINGRAYKIT_CHECK(gid != -1, SystemException("getpgid"));

		for (ThreadsMap::const_iterator it = g_threadsCreated.begin(); it != g_threadsCreated.end(); ++it)
		{
			u64 ptid = it->second->GetParentThread() ? it->second->GetParentThread()->GetTid() : 0;
			Stat stat;

			if (GetThreadStats(gid, it->first, stat))
				result.push_back(ThreadStats(it->first, ptid, it->second->GetThreadName(), TicksToMs(stat.utime), TicksToMs(stat.stimev),
						TicksToMs(it->second->GetChildrenUTime()), TicksToMs(it->second->GetChildrenSTime())));
			else
				result.push_back(ThreadStats(it->first, ptid, it->second->GetThreadName(), 999, 999, TicksToMs(it->second->GetChildrenUTime()), TicksToMs(it->second->GetChildrenSTime())));
		}

		return result;
	}


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
			if (S_ISDIR(st.st_mode) && GetThreadStats(gid, tid, stat))
			{
				std::string threadName(stat.tcomm);
				u64 childrenUTime = 0; //stat.cutime;
				u64 childrenSTime = 0; //stat.cstime;
				u64 parentId = 0; //stat.ppid;
				if (g_threadsCreated.find(tid) != g_threadsCreated.end())
				{
					threadName = g_threadsCreated[tid]->GetThreadName();
					childrenUTime = g_threadsCreated[tid]->GetChildrenUTime();
					childrenSTime = g_threadsCreated[tid]->GetChildrenSTime();
					if (g_threadsCreated[tid]->GetParentThread())
						parentId = g_threadsCreated[tid]->GetParentThread()->GetTid();
				}

				result.push_back(ThreadStats(tid, parentId, threadName, TicksToMs(stat.utime), TicksToMs(stat.stimev), TicksToMs(childrenUTime), TicksToMs(childrenSTime)));
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
