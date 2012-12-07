#ifndef STINGRAY_TOOLKIT_DISCONTINUITYREPAIRER_H
#define STINGRAY_TOOLKIT_DISCONTINUITYREPAIRER_H

#include <stingray/log/Logger.h>
#include <stingray/timer/ElapsedTime.h>
#include <stingray/timer/Time.h>
#include <stingray/toolkit/Backtrace.h>
#include <stingray/toolkit/optional.h>

namespace stingray {


	struct DiscontinuityRepairer // repairs discontinuities in both PTS and PCR
	{
	private:
		static NamedLogger	s_logger;
		ElapsedTime			_timeSinceLastIndex;
		optional<u64>		_firstIndex, _lastIndex;
		bool				_paused, _allowDiscontinuity;

	public:
		DiscontinuityRepairer() : _paused(false), _allowDiscontinuity(false)
		{}

		optional<TimeDuration> GetTimeSinceLast()
		{ return _firstIndex ? optional<TimeDuration>(TimeDuration(_timeSinceLastIndex.ElapsedMilliseconds())) : optional<TimeDuration>(); }

		void Pause(bool paused)
		{
			if (paused == _paused)
				return;

			if (paused)
			{
				_timeSinceLastIndex.Restart();
			}
			else
			{
				_timeSinceLastIndex.Restart();
			}
			_paused = paused;
		}

		void AllowDiscontinuiny()
		{
			_allowDiscontinuity = true;
		}

		void Reset()
		{
			_firstIndex = _lastIndex = null;
		}

		u64 Repair(u64 milliseconds, u64 discontinuityValue = 0)
		{
			if (_paused)
			{
				s_logger.Warning() << "Stream is paused!" << Backtrace().Get();
				return 0;
			}

			if (!_firstIndex)
			{
				_firstIndex = _lastIndex = milliseconds;
				_timeSinceLastIndex.Restart();
				return 0;
			}

			s64 time_since_last = _timeSinceLastIndex.ElapsedMilliseconds();
			if (time_since_last == 0)
				return _lastIndex.get();
			_timeSinceLastIndex.Restart();

			s64 gap = CalculateGap((s64)milliseconds - _lastIndex.get(), time_since_last, discontinuityValue);

			if (gap != 0 && !_allowDiscontinuity)
				_firstIndex = _firstIndex.get() - gap;

			_allowDiscontinuity = false;
			_lastIndex = milliseconds;
			return milliseconds - _firstIndex.get();
		}

		u64 RepairRelative(u64 milliseconds)
		{
			if (_paused)
			{
				s_logger.Warning() << "Stream is paused!" << Backtrace().Get();
				return 0;
			}

			if (!_lastIndex)
			{
				_lastIndex = milliseconds;
				_timeSinceLastIndex.Restart();
				return 0;
			}

			s64 time_since_last = _timeSinceLastIndex.ElapsedMilliseconds();
			if (time_since_last == 0)
				return 0;
			_timeSinceLastIndex.Restart();

			s64 gap = CalculateGap((s64)milliseconds - _lastIndex.get(), time_since_last);

			if (gap != 0 && !_allowDiscontinuity)
				_lastIndex = _lastIndex.get() - gap;

			_allowDiscontinuity = false;
			u64 result = milliseconds - _lastIndex.get();
			_lastIndex = milliseconds;
			return result;
		}

	private:
		static s64 CalculateGap(s64 stampDifference, u64 timeDifference, u64 hintDifference = 0)
		{
			if (stampDifference > 0)
			{
				s64 proportion = 1000 * stampDifference / timeDifference;
				if (500 < proportion && proportion < 2000)
					return 0;
			}

			return (hintDifference ? hintDifference : timeDifference) - stampDifference;
		}
	};


}


#endif

