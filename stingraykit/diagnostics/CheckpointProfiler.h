#ifndef STINGRAYKIT_DIAGNOSTICS_CHECKPOINTPROFILER_H
#define STINGRAYKIT_DIAGNOSTICS_CHECKPOINTPROFILER_H

// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <string>
#include <utility>
#include <vector>

#include <stingraykit/log/LogLevel.h>
#include <stingraykit/thread/Thread.h>
#include <stingraykit/time/ElapsedTime.h>
#include <stingraykit/time/Time.h>
#include <stingraykit/optional.h>


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
		STINGRAYKIT_DECLARE_PTR(CheckpointProfilerHolder);

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
