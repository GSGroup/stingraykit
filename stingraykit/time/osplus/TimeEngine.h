#ifndef STINGRAYKIT_TIME_OSPLUS_TIMEENGINE_H
#define STINGRAYKIT_TIME_OSPLUS_TIMEENGINE_H


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
