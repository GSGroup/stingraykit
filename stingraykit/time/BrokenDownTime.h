#ifndef STINGRAYKIT_TIME_BROKENDOWNTIME_H
#define STINGRAYKIT_TIME_BROKENDOWNTIME_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/Types.h>

#include <string>

namespace stingray
{

	/**
	 * @addtogroup toolkit_timer
	 * @{
	 */

	struct BrokenDownTime
	{
	private:
		class FormatMatcher;

	private:
		static FormatMatcher	s_formatMatcher;

	public:
		s16		Milliseconds;
		s16		Seconds;
		s16		Minutes;
		s16		Hours;
		s16		WeekDay;
		s16		MonthDay;
		s16		Month;
		s16		YearDay;
		s16		Year;

		BrokenDownTime()
			: Milliseconds(), Seconds(), Minutes(), Hours(), WeekDay(), MonthDay(1), Month(1), YearDay(1), Year(1970)
		{ }

		BrokenDownTime(s16 milliseconds, s16 seconds, s16 minutes, s16 hours, s16 weekDay, s16 monthDay, s16 month, s16 yearDay, s16 year)
			: Milliseconds(milliseconds), Seconds(seconds), Minutes(minutes), Hours(hours), WeekDay(weekDay), MonthDay(monthDay), Month(month), YearDay(yearDay), Year(year)
		{ }

		BrokenDownTime GetDayStart();

		int GetMaxDaysInMonth() const;
		static int GetMaxDaysInMonth(int year, int month);

		std::string ToString(const std::string& format = std::string()) const;
	};

	/** @} */

}

#endif
