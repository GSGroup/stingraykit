#ifndef STINGRAY_TOOLKIT_DIAGNOSTICS_CHECKPOINTPROFILER_H
#define STINGRAY_TOOLKIT_DIAGNOSTICS_CHECKPOINTPROFILER_H


#include <string>
#include <utility>
#include <vector>

#include <stingray/toolkit/log/LogLevel.h>
#include <stingray/toolkit/thread/Thread.h>
#include <stingray/toolkit/time/ElapsedTime.h>
#include <stingray/toolkit/time/Time.h>
#include <stingray/toolkit/optional.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_profiling
	 * @{
	 */

	class CheckpointProfiler
	{
		typedef std::vector<std::string>	CheckpointNames;
		typedef std::vector<std::pair<std::string, TimeDuration> >	CheckpointTimes;

	public:
		struct CheckpointEntry
		{
			std::string		Name;
			TimeDuration	DurationBefore;

			CheckpointEntry(const std::string& name, TimeDuration durationBefore);
			std::string ToString() const;
		};

		struct Result
		{
			typedef std::vector<CheckpointEntry>	CheckpointsVector;

			std::string				ProfilerName;
			CheckpointsVector		Checkpoints;

			std::string ToString() const;
		};

	private:
		std::string				_name;
		CheckpointTimes			_checkpointTimes;
		optional<ElapsedTime>	_elapsedTime;
		Mutex					_mutex;

	public:
		CheckpointProfiler(const std::string& name = "");

		void Checkpoint(const std::string& checkpointName = "");
		void Start();
		Result Stop();
	};


	class AutoCheckpointProfiler
	{
		class CheckpointProfilerHolder
		{
		public:
			CheckpointProfiler		_profiler;
			LogLevel				_logLevel;
			bool					_enabled;

			CheckpointProfilerHolder(const std::string& name, bool enabled);
			~CheckpointProfilerHolder();
		};
		TOOLKIT_DECLARE_PTR(CheckpointProfilerHolder);

	private:
		CheckpointProfilerHolderPtr		_profilerHolder;

	public:
		AutoCheckpointProfiler(const std::string& name = "", bool enabled = true);

		void Checkpoint(const std::string& checkpointName = "") const;
	};


	class CheckpointTracer
	{
		CheckpointProfiler&	_profiler;
		std::string			_message;
	public:
		CheckpointTracer(CheckpointProfiler& profiler, const std::string& message) : _profiler(profiler), _message(message)
		{ _profiler.Checkpoint(StringBuilder() % "Entered " % _message); }
		~CheckpointTracer()
		{ _profiler.Checkpoint(StringBuilder() % "Left " % _message); }
	};

	/** @} */

}


#endif
