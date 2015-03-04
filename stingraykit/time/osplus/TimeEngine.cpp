// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/time/osplus/TimeEngine.h>

#include <osplus.h>
#include <osplus/timer.h>

#include <sys/time.h>

#include <stingraykit/exception.h>


namespace stingray {
namespace osplus
{

	s64 TimeEngine::s_delta = 0;
	s16 TimeEngine::s_minutesFromUtc = 0;


	s64 TimeEngine::GetMillisecondsSinceEpoch()
	{ return ((s64)time_now() + s_delta) * 1000 / time_ticks_per_sec(); }

	void TimeEngine::SetMillisecondsSinceEpoch(s64 milliseconds)
	{ s_delta = (milliseconds * time_ticks_per_sec() / 1000) - time_now(); /* FIXME: interpolate delta? */ }

	s16 TimeEngine::GetMinutesFromUtc()
	{ return s_minutesFromUtc; }

	void TimeEngine::SetMinutesFromUtc(s16 minutes)
	{ s_minutesFromUtc = minutes; }

	s64 TimeEngine::MillisecondsFromBrokenDown(const BrokenDownTime& bdTime)
	{
		tm bdt = {};
		bdt.tm_sec = bdTime.Seconds;
		bdt.tm_min = bdTime.Minutes;
		bdt.tm_hour = bdTime.Hours;
		bdt.tm_mday = bdTime.MonthDay;
		bdt.tm_mon = bdTime.Month - 1;
		bdt.tm_year = bdTime.Year - 1900;
		time_t result = osplus_mktime(&bdt);
		STINGRAYKIT_CHECK(result != -1, Exception("mktime failed while processing bdt = " + bdTime.ToString() + "!"));
		return (s64)result * 1000;
	}

	BrokenDownTime TimeEngine::BrokenDownFromMilliseconds(s64 milliseconds)
	{
		time_t t = milliseconds / 1000;
		tm b = { };

		if (osplus_localtime(&t, &b) == NULL)
			STINGRAYKIT_THROW(Exception("localtime_r failed!"));

		return BrokenDownTime(milliseconds % 1000, b.tm_sec, b.tm_min, b.tm_hour, b.tm_wday, b.tm_mday, b.tm_mon + 1, b.tm_yday, b.tm_year + 1900);
	}

	MonotonicTimer TimeEngine::CreateMonotonicTimer()
	{ return MonotonicTimer(); }


}}
