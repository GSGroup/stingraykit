#ifndef STINGRAYKIT_TIME_POSIX_UTILS_H
#define STINGRAYKIT_TIME_POSIX_UTILS_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <time.h>

#include <stingraykit/time/Time.h>
#include <stingraykit/SystemException.h>


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
