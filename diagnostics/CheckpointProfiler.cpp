#include <stingray/toolkit/diagnostics/CheckpointProfiler.h>

#include <stingray/toolkit/log/Logger.h>
#include <stingray/toolkit/string/StringUtils.h>


namespace stingray
{

	CheckpointProfiler::CheckpointEntry::CheckpointEntry(const std::string& name, TimeDuration durationBefore)
		: Name(name), DurationBefore(durationBefore)
	{ }


	std::string CheckpointProfiler::CheckpointEntry::ToString() const
	{ return Name + " - " + DurationBefore.ToString(); }


	//////////////////////////////////////////////////////////////////////////////////


	std::string CheckpointProfiler::Result::ToString() const
	{
		StringBuilder sb;
		sb % "{ ";
		if (!ProfilerName.empty())
			sb % "profiler: " % ProfilerName % ", ";
		sb % "checkpoints: " % Checkpoints % " }";
		return sb;
	}


	//////////////////////////////////////////////////////////////////////////////////


	CheckpointProfiler::CheckpointProfiler(const std::string& name)
		: _name(name)
	{ }

	void CheckpointProfiler::Checkpoint(const std::string& checkpointName)
	{
		MutexLock l(_mutex);

		if (!_elapsedTime)
			return;

		std::string name = checkpointName.empty() ? "#" + ToString(_checkpointTimes.size()) : checkpointName;
		_checkpointTimes.push_back(std::make_pair(name, TimeDuration(_elapsedTime->ElapsedMilliseconds())));
	}


	void CheckpointProfiler::Start()
	{
		MutexLock l(_mutex);

		_checkpointTimes.clear();
		_elapsedTime = ElapsedTime();
	}


	CheckpointProfiler::Result CheckpointProfiler::Stop()
	{
		MutexLock l(_mutex);

		Result result;
		result.ProfilerName = _name;
		TimeDuration prev;
		for (CheckpointTimes::const_iterator it = _checkpointTimes.begin(); it != _checkpointTimes.end(); ++it)
		{
			result.Checkpoints.push_back(CheckpointEntry(it->first, it->second - prev));
			prev = it->second;
		}
		_elapsedTime.reset();
		_checkpointTimes.clear();
		return result;
	}


	//////////////////////////////////////////////////////////////////////////////////


	AutoCheckpointProfiler::CheckpointProfilerHolder::CheckpointProfilerHolder(const std::string& name, bool enabled)
		: _profiler(name), _logLevel(LogLevel::Info), _enabled(enabled)
	{
		_profiler.Start();
	}


	AutoCheckpointProfiler::CheckpointProfilerHolder::~CheckpointProfilerHolder()
	{
		//Logger::Stream(_logLevel) << _profiler.Stop();
		StringBuilder sb;
		_profiler.Checkpoint("stopping the profiler");
		CheckpointProfiler::Result res = _profiler.Stop();
		if (!_enabled)
			return;
		sb % "\n--- " % res.ProfilerName % " ---\n";
		TimeDuration t;
		for (CheckpointProfiler::Result::CheckpointsVector::const_iterator it = res.Checkpoints.begin(); it != res.Checkpoints.end(); ++it)
		{
			t += it->DurationBefore;
			sb % t % " | " % it->DurationBefore % " | " % it->Name % "\n";
		}
		Logger::Stream(_logLevel) << sb;
	}


	AutoCheckpointProfiler::AutoCheckpointProfiler(const std::string& name, bool enabled)
		: _profilerHolder(new CheckpointProfilerHolder(name, enabled))
	{ }


	void AutoCheckpointProfiler::Checkpoint(const std::string& checkpointName) const
	{ _profilerHolder->_profiler.Checkpoint(checkpointName); }

}
