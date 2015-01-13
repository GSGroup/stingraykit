#ifndef STINGRAYKIT_TIME_POSIX_TIMEENGINE_H
#define STINGRAYKIT_TIME_POSIX_TIMEENGINE_H


#include <time.h>
#include <sys/time.h>

#include <stingray/toolkit/thread/Thread.h>
#include <stingray/toolkit/time/BrokenDownTime.h>
#include <stingray/toolkit/Atomic.h>


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
		static s16		s_minutesFromUtc;
#ifndef PLATFORM_EMBEDDED
		static Mutex	s_deltaMillisecondsMutex;
		static s64		s_deltaMilliseconds;
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
