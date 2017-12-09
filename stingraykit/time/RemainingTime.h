#ifndef STINGRAYKIT_TIME_REMAININGTIME_H
#define STINGRAYKIT_TIME_REMAININGTIME_H

#include <stingraykit/time/ElapsedTime.h>

namespace stingray
{

	class RemainingTime
	{
	private:
		TimeDuration	_timeout;
		ElapsedTime		_elapsed;

	public:
		explicit RemainingTime(TimeDuration timeout) : _timeout(timeout) { }

		void Restart()					{ _elapsed.Restart(); }

		TimeDuration Remaining() const
		{
			const TimeDuration elapsed = _elapsed.Elapsed();
			return _timeout > elapsed ? _timeout - elapsed : TimeDuration();
		}

		bool Expired() const			{ return _timeout <= _elapsed.Elapsed(); }
	};

}

#endif
