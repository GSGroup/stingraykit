#ifndef STINGRAYKIT_TIME_ELAPSEDTIME_H
#define STINGRAYKIT_TIME_ELAPSEDTIME_H


#include <stingraykit/time/Time.h>
#include <stingraykit/time/TimeEngine.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_timer
	 * @{
	 */

	class ElapsedTime
	{
	private:
		MonotonicTimer		_timer;
		s64					_start;

	public:
		ElapsedTime();

		TimeDuration Restart();

		s64 ElapsedMilliseconds() const;
		s64 ElapsedMicroseconds() const;

		TimeDuration Elapsed() const;
	};

	/** @} */

}

#endif
