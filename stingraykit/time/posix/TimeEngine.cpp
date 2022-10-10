// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/time/posix/TimeEngine.h>

#include <stingraykit/string/ToString.h>
#include <stingraykit/thread/atomic/AtomicInt.h>
#include <stingraykit/SystemException.h>

#include <time.h>
#include <sys/time.h>

namespace stingray {
namespace posix
{

#ifdef STINGRAYKIT_32_BIT_TIME_T

	//work around year 2038 bug.
	//gmtime64_r and timegm64 implementations was taken from Bionic with some modifications

#	define LEAP_CHECK(n) ((!(((n) + 1900) % 400) || (!(((n) + 1900) % 4) && (((n) + 1900) % 100))) != 0)
#	define WRAP(a,b,m)	((a) = ((a) <  0  ) ? ((b)--, (a) + (m)) : (a))

	namespace
	{

		const int days_in_months[4][13] =
		{
			{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
			{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
			{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
			{0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366},
		};

		const int days_in_gregorian_cycle = ((365 * 400) + 100 - 4 + 1);


		struct tm *gmtime64_r(s64 t, struct tm *p)
		{
			int v_tm_sec, v_tm_min, v_tm_hour, v_tm_mon, v_tm_wday;
			s64 v_tm_tday;
			int leap;
			s64 m;
			s64 time = t;
			int year = 70;
			int cycles = 0;

			v_tm_sec =  (int)(time % 60);
			time /= 60;
			v_tm_min =  (int)(time % 60);
			time /= 60;
			v_tm_hour = (int)(time % 24);
			time /= 24;
			v_tm_tday = time;

			WRAP (v_tm_sec, v_tm_min, 60);
			WRAP (v_tm_min, v_tm_hour, 60);
			WRAP (v_tm_hour, v_tm_tday, 24);

			v_tm_wday = (int)((v_tm_tday + 4) % 7);
			if (v_tm_wday < 0)
				v_tm_wday += 7;
			m = v_tm_tday;

			if (m >= 0)
			{
				cycles = (int)(m / (s64) days_in_gregorian_cycle);
				m -= (cycles * (s64) days_in_gregorian_cycle);
				year += (cycles * 400);

				leap = LEAP_CHECK(year);
				while (m >= (s64) days_in_months[leap + 2][12])
				{
					m -= (s64) days_in_months[leap + 2][12];
					year++;
					leap = LEAP_CHECK(year);
				}

				v_tm_mon = 0;
				while (m >= (s64) days_in_months[leap][v_tm_mon])
				{
					m -= (s64) days_in_months[leap][v_tm_mon];
					v_tm_mon++;
				}
			}
			else
			{
				year--;

				cycles = (int)((m / (s64) days_in_gregorian_cycle) + 1);
				m -= (cycles * (s64) days_in_gregorian_cycle);
				year += (cycles * 400);

				leap = LEAP_CHECK(year);
				while (m < (s64) -days_in_months[leap + 2][12])
				{
					m += (s64) days_in_months[leap + 2][12];
					year--;
					leap = LEAP_CHECK(year);
				}

				v_tm_mon = 11;
				while (m < (s64) -days_in_months[leap][v_tm_mon])
				{
					m += (s64) days_in_months[leap][v_tm_mon];
					v_tm_mon--;
				}
				m += (s64) days_in_months[leap][v_tm_mon];
			}

			p->tm_year = year;
			p->tm_mday = (int) m + 1;
			p->tm_yday = days_in_months[leap + 2][v_tm_mon] + (int)m;
			p->tm_sec  = v_tm_sec;
			p->tm_min  = v_tm_min;
			p->tm_hour = v_tm_hour;
			p->tm_mon  = v_tm_mon;
			p->tm_wday = v_tm_wday;
			return p;
		}


		s64 timegm64(const struct tm *date)
		{
			s64 days = 0;
			s64 seconds = 0;
			int year;
			int orig_year = date->tm_year + date->tm_mon / 12;
			int cycles = 0;
			if (orig_year > 100)
			{
				cycles = (orig_year - 100) / 400;
				orig_year -= cycles * 400;
				days += (s64)cycles * days_in_gregorian_cycle;
			}
			else if (orig_year < -300)
			{
				cycles = (orig_year - 100) / 400;
				orig_year -= cycles * 400;
				days += (s64)cycles * days_in_gregorian_cycle;
			}

			if (orig_year > 70)
			{
				year = 70;
				while (year < orig_year)
				{
					days += days_in_months[LEAP_CHECK(year) + 2][12];
					year++;
				}
			}
			else if (orig_year < 70)
			{
				year = 69;
				do {
					days -= days_in_months[LEAP_CHECK(year) + 2][12];
					year--;
				}
				while (year >= orig_year);
			}
			days += days_in_months[LEAP_CHECK(orig_year) + 2][date->tm_mon % 12];
			days += date->tm_mday - 1;
			seconds = days * 60 * 60 * 24;
			seconds += date->tm_hour * 60 * 60;
			seconds += date->tm_min * 60;
			seconds += date->tm_sec;
			return seconds;
		}

	}

#endif

	namespace
	{

		struct AtomicsHolder
		{
			static AtomicS32::Type	s_minutesFromUtc;
#ifndef PLATFORM_EMBEDDED
			static AtomicS64::Type	s_deltaMilliseconds;
#endif
		};

		AtomicS32::Type		AtomicsHolder::s_minutesFromUtc	= 0;
#ifndef PLATFORM_EMBEDDED
		AtomicS64::Type		AtomicsHolder::s_deltaMilliseconds = 0;
#endif

	}


	void TimeEngine::SetMillisecondsSinceEpoch(s64 milliseconds)
	{
#ifdef PLATFORM_EMBEDDED
		struct timeval tv = { };
		tv.tv_sec = milliseconds / 1000;
		tv.tv_usec = (milliseconds % 1000) * 1000;
		STINGRAYKIT_CHECK(settimeofday(&tv, 0) == 0, SystemException("settimeofday"));
#else
		struct timeval tv = { };
		STINGRAYKIT_CHECK(gettimeofday(&tv, 0) == 0, SystemException("gettimeofday"));

		const s64 ms = (s64)tv.tv_sec * 1000 + tv.tv_usec / 1000;
		AtomicS64::Store(AtomicsHolder::s_deltaMilliseconds, milliseconds - ms);
#endif
	}


	s64 TimeEngine::GetMillisecondsSinceEpoch()
	{
		struct timeval tv = { };
		STINGRAYKIT_CHECK(gettimeofday(&tv, 0) == 0, SystemException("gettimeofday"));

		const s64 ms = (s64)tv.tv_sec * 1000 + tv.tv_usec / 1000;
#ifdef PLATFORM_EMBEDDED
		return ms;
#else
		return ms + AtomicS64::Load(AtomicsHolder::s_deltaMilliseconds);
#endif
	}


	s16 TimeEngine::GetMinutesFromUtc()
	{ return AtomicS32::Load(AtomicsHolder::s_minutesFromUtc); }


	void TimeEngine::SetMinutesFromUtc(s16 minutes)
	{ AtomicS32::Store(AtomicsHolder::s_minutesFromUtc, minutes); }


	s64 TimeEngine::MillisecondsFromBrokenDown(const BrokenDownTime& bdTime)
	{
		tm bdt = { };
		bdt.tm_sec = bdTime.Seconds;
		bdt.tm_min = bdTime.Minutes;
		bdt.tm_hour = bdTime.Hours;
		bdt.tm_mday = bdTime.MonthDay;
		bdt.tm_mon = bdTime.Month - 1;
		bdt.tm_year = bdTime.Year - 1900;
#ifdef STINGRAYKIT_32_BIT_TIME_T
		const s64 result = timegm64(&bdt);
		STINGRAYKIT_CHECK(result != -1, SystemException(StringBuilder() % "timegm64(" % bdTime % ")"));
#else
		const time_t result = timegm(&bdt);
		STINGRAYKIT_CHECK(result != -1, SystemException(StringBuilder() % "timegm(" % bdTime % ")"));
#endif
		return (s64)result * 1000 + bdTime.Milliseconds;
	}


	BrokenDownTime TimeEngine::BrokenDownFromMilliseconds(s64 milliseconds)
	{
		tm b = { };
#ifdef STINGRAYKIT_32_BIT_TIME_T
		STINGRAYKIT_CHECK(gmtime64_r(milliseconds / 1000, &b) != NULL, SystemException(StringBuilder() % "gmtime64_r(" % milliseconds % ")"));
#else
		const time_t t = milliseconds / 1000;
		STINGRAYKIT_CHECK(gmtime_r(&t, &b) != NULL, SystemException(StringBuilder() % "gmtime_r(" % milliseconds % ")"));
#endif

		return BrokenDownTime(milliseconds % 1000, b.tm_sec, b.tm_min, b.tm_hour, b.tm_wday, b.tm_mday, b.tm_mon + 1, b.tm_yday, b.tm_year + 1900);
	}


	u64 TimeEngine::GetMonotonicMicroseconds()
	{
#if POSIX_HAVE_MONOTONIC_TIMER
		clockid_t clock = CLOCK_MONOTONIC;
#else
		clockid_t clock = CLOCK_REALTIME;
#endif

		struct timespec ts = { };
		STINGRAYKIT_CHECK(clock_gettime(clock, &ts) != -1, SystemException("clock_gettime"));
		return (u64)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
	}


	u64 TimeEngine::GetMonotonicNanoseconds()
	{
#if POSIX_HAVE_MONOTONIC_TIMER
		clockid_t clock = CLOCK_MONOTONIC;
#else
		clockid_t clock = CLOCK_REALTIME;
#endif

		struct timespec ts = { };
		STINGRAYKIT_CHECK(clock_gettime(clock, &ts) != -1, SystemException("clock_gettime"));
		return (u64)ts.tv_sec * 1000000000 + ts.tv_nsec;
	}

}}
