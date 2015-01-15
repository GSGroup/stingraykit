#ifndef STINGRAYKIT_TIME_BROKENDOWNTIME_H
#define STINGRAYKIT_TIME_BROKENDOWNTIME_H


#include <string>

#include <stingraykit/Types.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_timer
	 * @{
	 */

	struct BrokenDownTime
	{
		s16		Milliseconds;
		s16		Seconds;
		s16		Minutes;
		s16		Hours;
		s16		WeekDay;
		s16		MonthDay;
		s16		Month;
		s16		YearDay;
		s16		Year;

		BrokenDownTime(): Milliseconds(), Seconds(), Minutes(), Hours(), WeekDay(), MonthDay(), Month(1), YearDay(1), Year(1970)
		{ }

		BrokenDownTime(s16 milliseconds, s16 seconds, s16 minutes, s16 hours, s16 weekDay, s16 monthDay, s16 month, s16 yearDay, s16 year) :
			Milliseconds(milliseconds), Seconds(seconds), Minutes(minutes), Hours(hours), WeekDay(weekDay), MonthDay(monthDay), Month(month), YearDay(yearDay), Year(year)
		{ }

		BrokenDownTime GetDayStart();
		int GetMaxDaysInMonth() const;
		static int GetMaxDaysInMonth(int year, int month);
		std::string ToString(const std::string& format = "") const;
	};

	/** @} */

}


#endif
