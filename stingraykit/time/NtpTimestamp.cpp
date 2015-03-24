#include <stingray/stingraykit/stingraykit/time/NtpTimestamp.h>

namespace stingray
{

	NtpTimestamp::NtpTimestamp(const u64 timestamp) : _timestamp(timestamp) { }


	u64 NtpTimestamp::GetNtpTimestamp() const { return _timestamp; }


	Time NtpTimestamp::ToTime() const
	{
		u32 integerPart = _timestamp >> 32;
		u64 seconds = integerPart - DifferenceBetweenUnixNtpTime;

		u32 fractionPart = _timestamp - integerPart;
		u64 milliSeconds = fractionPart / ((u64)1 << 32);

		return Time(seconds * 1000 + milliSeconds);
	};


	NtpTimestamp NtpTimestamp::FromTime(const Time& time)
	{
		u32 integerPart = time.GetMilliseconds() / 1000 +  DifferenceBetweenUnixNtpTime;
		u32 fractionPart = time.GetMilliseconds() / 1000 * ((u64)1 << 32);

		u64 timestamp = ((u64)integerPart << 32) | fractionPart;

		return NtpTimestamp(timestamp);
	}

}
