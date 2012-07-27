#ifndef __GS_STINGRAY_TOOLKIT_DISCONTINUITYREPAIRER_H__
#define __GS_STINGRAY_TOOLKIT_DISCONTINUITYREPAIRER_H__

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

			bool discontinuity = false;
			if (milliseconds < _lastIndex.get())
			{
				s_logger.Warning() << "Discontinuity - negative difference!";
				discontinuity = true;
			}
			else
			{
				s64 proportion = 1000 * (milliseconds - _lastIndex.get()) / time_since_last;
				if (!(500 < proportion && proportion < 2000))
				{
					s_logger.Warning() << "Discontinuity! Proportion: " << proportion;
					discontinuity = true;
				}
			}

			if (discontinuity && _allowDiscontinuity)
				s_logger.Info() << "Skipping allowed discontinuity";
			else if (discontinuity)
			{
				s64 gap = _lastIndex.get() + time_since_last - milliseconds;
				if (discontinuityValue != 0)
					gap = _lastIndex.get() + discontinuityValue - milliseconds;
				_firstIndex = _firstIndex.get() - gap;
			}
			_allowDiscontinuity = false;
			_lastIndex = milliseconds;
			return milliseconds - _firstIndex.get();
		}
	};


}


#endif

