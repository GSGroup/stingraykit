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

		TimeDuration Remaining() const	{ return _timeout - _elapsed.Elapsed(); }
		bool Expired() const			{ return Remaining() <= TimeDuration(); }
	};

}

#endif
