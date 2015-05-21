#ifndef STINGRAYKIT_TIME_POSIX_TIMEENGINE_H
#define STINGRAYKIT_TIME_POSIX_TIMEENGINE_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <time.h>
#include <sys/time.h>

#include <stingraykit/thread/Thread.h>
#include <stingraykit/time/BrokenDownTime.h>
#include <stingraykit/Atomic.h>


namespace stingray {
namespace posix
{

	class MonotonicTimer
	{
		clockid_t _clock;

	public:
		MonotonicTimer(clockid_t clock_id) : _clock(clock_id) {}

		u64 GetMicroseconds() const;
	};


	class TimeEngine
	{
	private:
		static atomic_int_type		s_minutesFromUtc;
#ifndef PLATFORM_EMBEDDED
		static atomic_int_type		s_deltaMilliseconds;
#endif

	public:
		static s64 GetMillisecondsSinceEpoch();
		static void SetMillisecondsSinceEpoch(s64 milliseconds);

		static s16 GetMinutesFromUtc();
		static void SetMinutesFromUtc(s16 minutes);

		static s64 MillisecondsFromBrokenDown(const BrokenDownTime& bdTime);
		static BrokenDownTime BrokenDownFromMilliseconds(s64 milliseconds);

		static MonotonicTimer CreateMonotonicTimer();
	};

}}

#endif
