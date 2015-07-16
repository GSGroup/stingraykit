#ifndef STINGRAYKIT_DIAGNOSTICS_ELAPSEDTHREADTIME_H
#define STINGRAYKIT_DIAGNOSTICS_ELAPSEDTHREADTIME_H

#include <stingraykit/thread/Thread.h>

namespace stingray
{

	class ThreadTime
	{
	private:
		TimeDuration	_realTime;
		TimeDuration	_userTime;
		TimeDuration 	_systemTime;

	public:
		ThreadTime(const TimeDuration& realTime, const TimeDuration& userTime, const TimeDuration& systemTime)
			: _realTime(realTime), _userTime(userTime), _systemTime(systemTime)
		{ }

		TimeDuration GetRealTime() const { return _realTime; }
		TimeDuration GetUserTime() const { return _userTime; }
		TimeDuration GetSystemTime() const { return _systemTime; }

		std::string ToString() const { return StringBuilder() % "{ real: " % _realTime % ", user: " % _userTime % ", sys: " % _systemTime % " }"; }
	};


	class ElapsedThreadTime
	{
	private:
		ThreadId		_threadId;
		ElapsedTime		_realTime;
		ThreadCpuStats	_threadStats;

	public:
		ElapsedThreadTime()
			: _threadId(Thread::GetCurrentThreadId())
		{ _threadStats = GetThreadStats(); }

		ThreadTime Elapsed() const
		{
			const ThreadCpuStats stats = GetThreadStats();

			return ThreadTime(_realTime.Elapsed(), stats.GetUserTime() - _threadStats.GetUserTime(), stats.GetSystemTime() - _threadStats.GetSystemTime());
		}

	private:
		ThreadCpuStats GetThreadStats() const
		{
			Thread::ThreadStatsVec stats = Thread::GetStats();

			Thread::ThreadStatsVec::iterator it = std::find_if(stats.begin(), stats.end(), CompareMemberToValue<std::equal_to>(&ThreadStats::GetThreadId, _threadId));
			STINGRAYKIT_ASSERT(it != stats.end());

			return (*it).GetCpuStats();
		}
	};

}

#endif
