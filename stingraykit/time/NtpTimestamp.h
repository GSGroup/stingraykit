#ifndef STINGRAYKIT_TIME_NTPTIMESTAMP_H
#define STINGRAYKIT_TIME_NTPTIMESTAMP_H

#include <stingraykit/time/Time.h>

namespace stingray
{

	class NtpTimestamp
	{
		static const s64 DifferenceBetweenUnixNtpTime = 2208988800ll;
	private:
		u64 _timestamp;

	public:
		NtpTimestamp(const u64 timestamp);

		u64 GetNtpTimestamp() const;

		Time ToTime() const;
		static NtpTimestamp FromTime(const Time& time);
	};

}

#endif
