#ifndef STINGRAY_TOOLKIT_DIAGNOSTICS_ASYNCPROFILER_H
#define STINGRAY_TOOLKIT_DIAGNOSTICS_ASYNCPROFILER_H


#include <string>

#include <stingray/timer/ElapsedTime.h>
#include <stingray/timer/Timer.h>
#include <stingray/toolkit/shared_ptr.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_profiling
	 * @{
	 */

	class AsyncProfiler;
	TOOLKIT_DECLARE_PTR(AsyncProfiler);

	class AsyncProfiler
	{
	private:
		typedef function<std::string()>	NameGetterFunc;

		class CallInfo
		{
			optional<std::string>		_name;
			optional<NameGetterFunc>	_nameGetter;
			Backtrace					_backtrace;

		public:
			CallInfo(const optional<std::string>& name, const optional<NameGetterFunc>& nameGetter)
				: _name(name), _nameGetter(nameGetter)
			{ }

			std::string GetName()
			{
				if (!_name.is_initialized())
					_name = _nameGetter.get()();
				return *_name;
			}

			std::string GetBacktrace() const
			{ return _backtrace.Get(); }
		};
		TOOLKIT_DECLARE_PTR(CallInfo);

	public:
		class Session
		{
		public:
			struct Behaviour
			{
				TOOLKIT_ENUM_VALUES(Silent, Verbose);
				TOOLKIT_DECLARE_ENUM_CLASS(Behaviour);
			};

			struct NameGetterTag { };

		private:
			AsyncProfilerWeakPtr		_profiler;
			IThreadInfoPtr				_threadInfo;
			CallInfoPtr					_callInfo;
			TimerConnectionHolder		_criticalConnection;
			TimerConnectionHolder		_errorConnection;
			TimerConnectionHolder		_startConnection;
			ElapsedTime					_elapsed;
			Behaviour					_behaviour;

		public:
			Session(const AsyncProfilerWeakPtr& profiler, const std::string& name, size_t criticalMs, Behaviour behaviour = Behaviour::Silent);
			Session(const AsyncProfilerWeakPtr& profiler, const NameGetterFunc& nameGetter, size_t criticalMs, Behaviour behaviour, const NameGetterTag&);
			~Session();

		private:
			void Start(size_t criticalMs);
		};
		TOOLKIT_DECLARE_PTR(Session);

	private:
		static NamedLogger		s_logger;
		Timer					_timer;

	public:
		AsyncProfiler(const std::string& threadName);

	private:
		static void ReportStart(const CallInfoPtr& callInfo);
		static void ReportEnd(const CallInfoPtr& callInfo, TimeDuration time);
		static void ReportCriticalTime(const CallInfoPtr& callInfo, TimeDuration criticalTime, const IThreadInfoPtr& threadInfo);
		static void ReportErrorTime(const CallInfoPtr& callInfo, const IThreadInfoPtr& threadInfo, const shared_ptr<int>& counter);
	};


#define STINGRAY_PROFILE_CALL(Profiler_, Milliseconds_, Call_) \
	do { \
		AsyncProfiler::Session detail_session(Profiler_, #Call_, Milliseconds_); \
		Call_; \
	} while (false)

	/** @} */

}


#endif

