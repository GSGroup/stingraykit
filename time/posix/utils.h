#ifndef STINGRAYKIT_TIME_POSIX_UTILS_H
#define STINGRAYKIT_TIME_POSIX_UTILS_H

#include <time.h>

#include <stingray/toolkit/time/Time.h>
#include <stingray/toolkit/SystemException.h>


namespace stingray {
namespace posix
{

	inline void timespec_add(timespec* t, TimeDuration duration)
	{
		s64 us = duration.GetMicroseconds();
		if (us < 1000000)
			t->tv_nsec += us * 1000;
		else
		{
			t->tv_sec += us / 1000000;
			t->tv_nsec += (us % 1000000) * 1000;
		}

		if (t->tv_nsec >= 1000000000)
		{
			t->tv_sec += t->tv_nsec / 1000000000;
			t->tv_nsec %= 1000000000;
		}
	}

	inline void timespec_add(timespec* t, const timespec* summand)
	{
		t->tv_sec += summand->tv_sec;
		t->tv_nsec += summand->tv_nsec;
		for (; t->tv_nsec >= 1000000000; t->tv_nsec -= 1000000000)
			++t->tv_sec;
	}

	inline void timespec_sub(timespec* minuend, const timespec* subtrahend)
	{
		if (minuend->tv_nsec < subtrahend->tv_nsec)
		{
			minuend->tv_sec  -= 1;
			minuend->tv_nsec += 1000000000;
		}
		minuend->tv_sec  -= subtrahend->tv_sec;
		minuend->tv_nsec -= subtrahend->tv_nsec;
	}

	inline void timespec_now(clockid_t clockId, timespec* t)
	{ STINGRAYKIT_CHECK(clock_gettime(clockId, t) == 0, SystemException("clock_gettime")); }

}}


#endif
