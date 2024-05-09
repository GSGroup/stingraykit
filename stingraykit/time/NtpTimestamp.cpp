// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/time/NtpTimestamp.h>

namespace stingray
{

	Time NtpTimestamp::ToTime() const
	{
		u32 integerPart = _timestamp >> 32;
		u64 seconds = integerPart - DifferenceBetweenUnixNtpTime;

		u32 fractionPart = _timestamp - integerPart;
		u64 milliSeconds = fractionPart / ((u64)1 << 32);

		return Time(seconds * 1000 + milliSeconds);
	};


	NtpTimestamp NtpTimestamp::FromTime(Time time)
	{
		u32 integerPart = time.GetSeconds() +  DifferenceBetweenUnixNtpTime;
		u32 fractionPart = time.GetSeconds() * ((u64)1 << 32);

		u64 timestamp = ((u64)integerPart << 32) | fractionPart;

		return NtpTimestamp(timestamp);
	}

}
