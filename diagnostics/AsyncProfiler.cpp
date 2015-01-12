#include <stingray/toolkit/diagnostics/AsyncProfiler.h>

namespace stingray
{

	static const size_t MaxSessionTime = 60000;

	AsyncProfiler::Session::Session(const AsyncProfilerWeakPtr& profiler, const std::string& name, size_t criticalMs, Behaviour behaviour)
		: _profiler(profiler), _threadInfo(Thread::GetCurrentThreadInfo()), _callInfo(new CallInfo(name, null)), _behaviour(behaviour)
	{ Start(criticalMs); }


	AsyncProfiler::Session::Session(const AsyncProfilerWeakPtr& profiler, const NameGetterFunc& nameGetter, size_t criticalMs, Behaviour behaviour, const NameGetterTag&)
		: _profiler(profiler), _threadInfo(Thread::GetCurrentThreadInfo()), _callInfo(new CallInfo(null, nameGetter)), _behaviour(behaviour)
	{ Start(criticalMs); }


	void AsyncProfiler::Session::Start(size_t criticalMs)
	{
		AsyncProfilerPtr profiler = _profiler.lock();
		if (!profiler)
		{
			s_logger.Warning() << "Start called on dead profiler";
			return;
		}
		if (_behaviour == Behaviour::Verbose)
			profiler->ReportStart(_callInfo);
		_criticalConnection = profiler->_timer.SetTimeout(criticalMs, bind(&AsyncProfiler::ReportCriticalTime, _callInfo, TimeDuration(criticalMs), _threadInfo));
		_errorConnection = profiler->_timer.SetTimer(MaxSessionTime, bind(&AsyncProfiler::ReportErrorTime, _callInfo, _threadInfo, make_shared<int>(1)));
	}


	AsyncProfiler::Session::~Session()
	{
		AsyncProfilerPtr profiler = _profiler.lock();
		_criticalConnection.reset();
		_errorConnection.reset();
		if (!profiler)
		{
			s_logger.Warning() << "profiler session destroyed after profiler death";
			return;
		}
		if (_behaviour == Behaviour::Verbose)
			profiler->ReportEnd(_callInfo, _elapsed.Elapsed());
	}


	STINGRAYKIT_DEFINE_NAMED_LOGGER(AsyncProfiler);

	AsyncProfiler::AsyncProfiler(const std::string& threadName)
		: _timer(threadName, &Timer::DefaultExceptionHandler, false)
	{ }


	void AsyncProfiler::ReportStart(const CallInfoPtr& callInfo)
	{ s_logger.Info() << "Executing " << callInfo->GetName() << "..."; }

	void AsyncProfiler::ReportEnd(const CallInfoPtr& callInfo, TimeDuration time)
	{ s_logger.Info() << callInfo->GetName() << " took " << time; }

	void AsyncProfiler::ReportCriticalTime(const CallInfoPtr& callInfo, TimeDuration criticalTime, const IThreadInfoPtr& threadInfo)
	{
		s_logger.Warning() << callInfo->GetName() << (threadInfo? (" in thread '" + threadInfo->GetName() + "'"): std::string()) << " is being executed for more than " << criticalTime << "! Invoked from:\n" << callInfo->GetBacktrace();
		if (threadInfo)
			threadInfo->RequestBacktrace();
	}

	void AsyncProfiler::ReportErrorTime(const CallInfoPtr& callInfo, const IThreadInfoPtr& threadInfo, const shared_ptr<int>& counter)
	{
		s_logger.Error() << callInfo->GetName() << (threadInfo? (" in thread '" + threadInfo->GetName() + "'"): std::string()) << " is being executed for more than " << TimeDuration((s64)((*counter)++) * (s64)MaxSessionTime) << "! Invoked from:\n" << callInfo->GetBacktrace();
		if (threadInfo)
			threadInfo->RequestBacktrace();
	}

}
