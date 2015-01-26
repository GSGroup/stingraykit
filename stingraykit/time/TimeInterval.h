#ifndef STINGRAY_STINGRAYKIT_STINGRAYKIT_TIME_TIMEINTERVAL_H
#define STINGRAY_STINGRAYKIT_STINGRAYKIT_TIME_TIMEINTERVAL_H


#include <stingraykit/NumericRange.h>
#include <stingraykit/time/Time.h>

namespace stingray
{

	class TimeInterval : public NumericRange<TimeInterval, Time>
	{
	public:
		typedef NumericRange<TimeInterval, Time> base;

	public:
		TimeInterval() : base(Time(0), Time(0))
		{ }

		TimeInterval(Time start, Time end) : base(start, end)
		{ }

		TimeDuration GetDuration() const	{ return GetEnd() - GetStart(); }
		static TimeInterval CreateEmpty()	{ return TimeInterval(); }
	};

}

#endif
