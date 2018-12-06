// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/diagnostics/CheckpointProfiler.h>

#include <stingraykit/log/Logger.h>
#include <stingraykit/string/ToString.h>


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
		_checkpointTimes.push_back(std::make_pair(name, _elapsedTime->Elapsed()));
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
		: _profilerHolder(make_shared<CheckpointProfilerHolder>(name, enabled))
	{ }


	void AutoCheckpointProfiler::Checkpoint(const std::string& checkpointName) const
	{ _profilerHolder->_profiler.Checkpoint(checkpointName); }

}
