#ifndef STINGRAYKIT_TIME_OSPLUS_TIMEENGINE_H
#define STINGRAYKIT_TIME_OSPLUS_TIMEENGINE_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <os21/ostime.h>

#include <stingraykit/time/BrokenDownTime.h>


namespace stingray {
namespace osplus
{

	class MonotonicTimer
	{
	public:
		u64 GetMicroseconds() const //some random value from unspecified point in the past
		{ return (u64)time_now() * 1000000 / time_ticks_per_sec(); }
	};


	class TimeEngine
	{
	private:
		static s64 s_delta; //system time delta
		static s16 s_minutesFromUtc;

	public:
		TimeEngine();

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
