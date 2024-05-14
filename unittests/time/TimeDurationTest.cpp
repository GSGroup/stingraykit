// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/time/Time.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace 
{

	const s64 MillisecondsPerSecond	= 1000;
	const s64 MillisecondsPerMinute = 60 * MillisecondsPerSecond;
	const s64 MillisecondsPerHour	= 60 * MillisecondsPerMinute;
	const s64 MillisecondsPerDay	= 24 * MillisecondsPerHour;
	const s64 DaysPerWeek			= 7;
	
}

TEST(TimeDurationTest, ToMeasurmentUnit)
{
	ASSERT_EQ(TimeDuration::Second(), TimeDuration(1000));
	ASSERT_EQ(TimeDuration::Minute(), TimeDuration(60 * 1000));
	ASSERT_EQ(TimeDuration::Hour(), TimeDuration(60 * 60 * 1000));
	ASSERT_EQ(TimeDuration::Day(), TimeDuration(24 * 60 * 60 * 1000));
}


TEST(TimeDurationTest, FromMeasurmentUnit)
{
	ASSERT_EQ(TimeDuration::FromSeconds(3), TimeDuration(3 * 1000));
	ASSERT_EQ(TimeDuration::FromMinutes(3), TimeDuration(3 * 60 * 1000));
	ASSERT_EQ(TimeDuration::FromHours(3), TimeDuration(3 * 60 * 60 * 1000));
	ASSERT_EQ(TimeDuration::FromDays(3), TimeDuration(3 * 24 * 60 * 60 * 1000));
}


TEST(TimeDurationTest, Absolute)
{
	ASSERT_EQ(TimeDuration().Absolute(), TimeDuration());

	ASSERT_EQ(TimeDuration(1).Absolute(), TimeDuration(1));
	ASSERT_EQ(TimeDuration(-1).Absolute(), TimeDuration(1));

	ASSERT_EQ(TimeDuration::Second().Absolute(), TimeDuration::Second());
	ASSERT_EQ((-TimeDuration::Second()).Absolute(), TimeDuration::Second());

	ASSERT_EQ(TimeDuration::Max().Absolute(), TimeDuration::Max());
	ASSERT_ANY_THROW(TimeDuration::Min().Absolute());
}


TEST(TimeDurationTest, Round)
{
	ASSERT_EQ(TimeDuration::FromMicroseconds(0).RoundToMilliseconds(), TimeDuration::FromMicroseconds(0));

	ASSERT_EQ(TimeDuration::FromMicroseconds(1).RoundToMilliseconds(), TimeDuration::FromMicroseconds(0));
	ASSERT_EQ(TimeDuration::FromMicroseconds(-1).RoundToMilliseconds(), TimeDuration::FromMicroseconds(0));

	ASSERT_EQ(TimeDuration::FromMicroseconds(499).RoundToMilliseconds(), TimeDuration::FromMicroseconds(0));
	ASSERT_EQ(TimeDuration::FromMicroseconds(-499).RoundToMilliseconds(), TimeDuration::FromMicroseconds(0));

	ASSERT_EQ(TimeDuration::FromMicroseconds(500).RoundToMilliseconds(), TimeDuration::FromMicroseconds(1000));
	ASSERT_EQ(TimeDuration::FromMicroseconds(-500).RoundToMilliseconds(), TimeDuration::FromMicroseconds(-1000));

	ASSERT_EQ(TimeDuration::FromMicroseconds(999).RoundToMilliseconds(), TimeDuration::FromMicroseconds(1000));
	ASSERT_EQ(TimeDuration::FromMicroseconds(-999).RoundToMilliseconds(), TimeDuration::FromMicroseconds(-1000));

	ASSERT_EQ(TimeDuration::FromMicroseconds(1000).RoundToMilliseconds(), TimeDuration::FromMicroseconds(1000));
	ASSERT_EQ(TimeDuration::FromMicroseconds(-1000).RoundToMilliseconds(), TimeDuration::FromMicroseconds(-1000));
}


TEST(TimeDurationTest, ToString)
{
	ASSERT_EQ(TimeDuration::Max().ToString(), "2562047788:00:54.775");
	ASSERT_EQ(TimeDuration::Max().ToString(), TimeDuration::Max().ToString("hh:mm:ss.lll"));

//	ASSERT_EQ(TimeDuration::Min().ToString(), "-2562047788:00:54.775");
//	ASSERT_EQ(TimeDuration::Min().ToString(), TimeDuration::Min().ToString("hh:mm:ss.lll"));

	ASSERT_EQ(TimeDuration(1).ToString(), "00:00:00.001");
	ASSERT_EQ(TimeDuration(1).ToString("lll"), "001");
	ASSERT_EQ(TimeDuration(999).ToString(), "00:00:00.999");
	ASSERT_EQ(TimeDuration(999).ToString("lll"), "999");

	ASSERT_EQ(TimeDuration(MillisecondsPerSecond).ToString(), "00:00:01.000");
	ASSERT_EQ(TimeDuration(MillisecondsPerSecond).ToString("ss"), "01");
	ASSERT_EQ(TimeDuration(MillisecondsPerSecond).ToString("s"), "1");
	ASSERT_EQ(TimeDuration(MillisecondsPerSecond * 59).ToString(), "00:00:59.000");
	ASSERT_EQ(TimeDuration(MillisecondsPerSecond * 59).ToString("ss"), "59");
	ASSERT_EQ(TimeDuration(MillisecondsPerSecond * 59).ToString("s"), "59");

	ASSERT_EQ(TimeDuration(MillisecondsPerMinute).ToString(), "00:01:00.000");
	ASSERT_EQ(TimeDuration(MillisecondsPerMinute).ToString("mm"), "01");
	ASSERT_EQ(TimeDuration(MillisecondsPerMinute).ToString("m"), "1");
	ASSERT_EQ(TimeDuration(MillisecondsPerMinute * 59).ToString(), "00:59:00.000");
	ASSERT_EQ(TimeDuration(MillisecondsPerMinute * 59).ToString("mm"), "59");
	ASSERT_EQ(TimeDuration(MillisecondsPerMinute * 59).ToString("m"), "59");

	ASSERT_EQ(TimeDuration(MillisecondsPerHour).ToString(), "01:00:00.000");
	ASSERT_EQ(TimeDuration(MillisecondsPerHour).ToString("hh"), "01");
	ASSERT_EQ(TimeDuration(MillisecondsPerHour).ToString("h"), "1");
	ASSERT_EQ(TimeDuration(MillisecondsPerHour * 999).ToString(), "999:00:00.000");
	ASSERT_EQ(TimeDuration(MillisecondsPerHour * 999).ToString("hh"), "999");
	ASSERT_EQ(TimeDuration(MillisecondsPerHour * 999).ToString("h"), "999");

	ASSERT_EQ(TimeDuration(MillisecondsPerHour + MillisecondsPerMinute + MillisecondsPerSecond + 1).ToString(), "01:01:01.001");
	ASSERT_EQ(TimeDuration(MillisecondsPerHour + MillisecondsPerMinute + MillisecondsPerSecond + 1).ToString("h:m:s.lll"), "1:1:1.001");
	ASSERT_EQ(TimeDuration(MillisecondsPerHour * 999 + MillisecondsPerMinute * 59 + MillisecondsPerSecond * 59 + 999).ToString(), "999:59:59.999");
	ASSERT_EQ(TimeDuration(MillisecondsPerHour * 999 + MillisecondsPerMinute * 59 + MillisecondsPerSecond * 59 + 999).ToString("h:m:s.lll"), "999:59:59.999");
}


TEST(TimeDurationTest, FromIso8601)
{
	ASSERT_EQ(TimeDurationUtility::FromIso8601("P3D"), TimeDuration::FromDays(3));
	ASSERT_EQ(TimeDurationUtility::FromIso8601("P3M"), TimeDuration::FromDays(31 + 28 + 31));
	ASSERT_EQ(TimeDurationUtility::FromIso8601("P3Y"), TimeDuration::FromDays(3 * 365 + 1));

	ASSERT_EQ(TimeDurationUtility::FromIso8601("PT3S"), TimeDuration::FromSeconds(3));
	ASSERT_EQ(TimeDurationUtility::FromIso8601("PT3M"), TimeDuration::FromMinutes(3));
	ASSERT_EQ(TimeDurationUtility::FromIso8601("PT3H"), TimeDuration::FromHours(3));

	ASSERT_EQ(TimeDurationUtility::FromIso8601("P3W"), TimeDuration::FromDays(3 * DaysPerWeek));

	ASSERT_EQ(TimeDurationUtility::FromIso8601("PT3M3S"), TimeDuration::FromMinutes(3) + TimeDuration::FromSeconds(3));
	ASSERT_EQ(TimeDurationUtility::FromIso8601("PT3H3M"), TimeDuration::FromHours(3) + TimeDuration::FromMinutes(3));
	ASSERT_EQ(TimeDurationUtility::FromIso8601("PT3H3M3S"), TimeDuration::FromHours(3) + TimeDuration::FromMinutes(3) + TimeDuration::FromSeconds(3));

	ASSERT_EQ(TimeDurationUtility::FromIso8601("P3M3D"), TimeDuration::FromDays(31 + 28 + 31) + TimeDuration::FromDays(3));
	ASSERT_EQ(TimeDurationUtility::FromIso8601("P3Y3M"), TimeDuration::FromDays(3 * 365 + 1) + TimeDuration::FromDays(31 + 28 + 31));
	ASSERT_EQ(TimeDurationUtility::FromIso8601("P3Y3M3D"), TimeDuration::FromDays(3 * 365 + 1) + TimeDuration::FromDays(31 + 28 + 31) + TimeDuration::FromDays(3));
}


TEST(TimeDurationTest, FromIso8601_Zero)
{
	ASSERT_EQ(TimeDurationUtility::FromIso8601("PT0S"), TimeDuration(0));
	ASSERT_EQ(TimeDurationUtility::FromIso8601("PT0M"), TimeDuration(0));
	ASSERT_EQ(TimeDurationUtility::FromIso8601("PT0H"), TimeDuration(0));

	ASSERT_EQ(TimeDurationUtility::FromIso8601("P0D"), TimeDuration(0));
	ASSERT_EQ(TimeDurationUtility::FromIso8601("P0M"), TimeDuration(0));
	ASSERT_EQ(TimeDurationUtility::FromIso8601("P0Y"), TimeDuration(0));

	ASSERT_EQ(TimeDurationUtility::FromIso8601("P0DT0S"), TimeDuration(0));
}


TEST(TimeDurationTest, FromIso8601_CarryOver)
{
	ASSERT_EQ(TimeDurationUtility::FromIso8601("PT60S"), TimeDuration::FromMinutes(1));
	ASSERT_EQ(TimeDurationUtility::FromIso8601("PT125S"), TimeDuration::FromMinutes(2) + TimeDuration::FromSeconds(5));
	ASSERT_EQ(TimeDurationUtility::FromIso8601("PT3725S"), TimeDuration::FromHours(1) + TimeDuration::FromMinutes(2) + TimeDuration::FromSeconds(5));
	ASSERT_EQ(TimeDurationUtility::FromIso8601("PT1502M"), TimeDuration::FromDays(1) + TimeDuration::FromHours(1) + TimeDuration::FromMinutes(2));

	ASSERT_EQ(TimeDurationUtility::FromIso8601("P17D"), TimeDuration::FromDays(2 * DaysPerWeek) + TimeDuration::FromDays(3));
	ASSERT_EQ(TimeDurationUtility::FromIso8601("P367D"), TimeDuration::FromDays(367));

	ASSERT_EQ(TimeDurationUtility::FromIso8601("P60W"), TimeDuration::FromDays(60 * DaysPerWeek));

	ASSERT_EQ(TimeDurationUtility::FromIso8601("P60M"), TimeDuration::FromDays(5 * 365 + 1));
	ASSERT_EQ(TimeDurationUtility::FromIso8601("P63M"), TimeDuration::FromDays(5 * 365 + 1) + TimeDuration::FromDays(31 + 28 + 31));
}


TEST(TimeDurationTest, FromIso8601_LowerCase)
{
	ASSERT_EQ(TimeDurationUtility::FromIso8601("p3d"), TimeDuration::FromDays(3));
	ASSERT_EQ(TimeDurationUtility::FromIso8601("p3m"), TimeDuration::FromDays(31 + 28 + 31));
	ASSERT_EQ(TimeDurationUtility::FromIso8601("p3y"), TimeDuration::FromDays(3 * 365 + 1));

	ASSERT_EQ(TimeDurationUtility::FromIso8601("pt3s"), TimeDuration::FromSeconds(3));
	ASSERT_EQ(TimeDurationUtility::FromIso8601("pt3m"), TimeDuration::FromMinutes(3));
	ASSERT_EQ(TimeDurationUtility::FromIso8601("pt3h"), TimeDuration::FromHours(3));

	ASSERT_EQ(TimeDurationUtility::FromIso8601("P3w"), TimeDuration::FromDays(3 * DaysPerWeek));
}


TEST(TimeDurationTest, FromIso8601_BaseTime)
{
	ASSERT_EQ(TimeDurationUtility::FromIso8601("P3D", Time(365 * MillisecondsPerDay)), TimeDuration::FromDays(3));
	ASSERT_EQ(TimeDurationUtility::FromIso8601("P3M", Time(365 * MillisecondsPerDay)), TimeDuration::FromDays(31 + 28 + 31));
	ASSERT_EQ(TimeDurationUtility::FromIso8601("P3M", Time(2 * 365 * MillisecondsPerDay)), TimeDuration::FromDays(31 + 29 + 31));
	ASSERT_EQ(TimeDurationUtility::FromIso8601("P3Y", Time((3 * 365 + 1) * MillisecondsPerDay)).ToString(), TimeDuration::FromDays(3 * 365).ToString());

	ASSERT_EQ(TimeDurationUtility::FromIso8601("PT3S", Time(365 * MillisecondsPerDay)), TimeDuration::FromSeconds(3));
	ASSERT_EQ(TimeDurationUtility::FromIso8601("PT3M", Time(365 * MillisecondsPerDay)), TimeDuration::FromMinutes(3));
	ASSERT_EQ(TimeDurationUtility::FromIso8601("PT3H", Time(365 * MillisecondsPerDay)), TimeDuration::FromHours(3));

	ASSERT_EQ(TimeDurationUtility::FromIso8601("P3W", Time(365 * MillisecondsPerDay)), TimeDuration::FromDays(3 * DaysPerWeek));

	ASSERT_EQ(TimeDurationUtility::FromIso8601("PT3M3S", Time(365 * MillisecondsPerDay)), TimeDuration::FromMinutes(3) + TimeDuration::FromSeconds(3));
	ASSERT_EQ(TimeDurationUtility::FromIso8601("PT3H3M3S", Time(365 * MillisecondsPerDay)), TimeDuration::FromHours(3) + TimeDuration::FromMinutes(3) + TimeDuration::FromSeconds(3));
	ASSERT_EQ(TimeDurationUtility::FromIso8601("PT3M3S", Time(365 * MillisecondsPerDay)), TimeDuration::FromMinutes(3) + TimeDuration::FromSeconds(3));

	ASSERT_EQ(TimeDurationUtility::FromIso8601("P1YT50M", Time(50 * MillisecondsPerMinute)), TimeDuration::FromDays(365) + TimeDuration::FromMinutes(50));
	ASSERT_EQ(TimeDurationUtility::FromIso8601("P1YT15H", Time(15 * MillisecondsPerHour)), TimeDuration::FromDays(365) + TimeDuration::FromHours(15));

	ASSERT_EQ(TimeDurationUtility::FromIso8601("P1YT50S", Time(50 * MillisecondsPerSecond)), TimeDuration::FromDays(365) + TimeDuration::FromSeconds(50));
	ASSERT_EQ(TimeDurationUtility::FromIso8601("P1YT50S", Time(2 * 365 * MillisecondsPerDay + 50 * MillisecondsPerSecond)), TimeDuration::FromSeconds(50) + TimeDuration::FromDays(366));
}


TEST(TimeDurationTest, FromIso8601_Substring)
{
	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("textPT1S"));
	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("PT1Stext"));
	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("textPT1Stext"));
}


TEST(TimeDurationTest, FromIso8601_Empty)
{
	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601(""));
	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("P"));
	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("PT"));
	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("P3DT"));
}


TEST(TimeDurationTest, FromIso8601_WrongOrder)
{
	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("P3D3M"));
	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("3M3Y"));
	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("P3D3M3Y"));
	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("P3D3Y3M"));
	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("P3M3Y3D"));

	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("PT3S3M"));
	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("PT3M3H"));
	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("PT3S3M3H"));
	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("PT3M3H3S"));
	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("PT3H3S3M"));
}


TEST(TimeDurationTest, FromIso8601_BadFormat)
{
	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("PT3D"));
	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("P3S"));
	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("P3B"));
	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("P33"));

	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("T3S"));
	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("3D"));

	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("PTS"));
	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("PD"));

	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("P3W3D"));
	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("P3D3W"));
	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("P3WT3S"));

	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("P3D3D"));
	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("PT3S3S"));

	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("P3DT3MT3S"));

	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("PT111111111111111111111111111111111111111S"));
}


TEST(TimeDurationTest, FromIso8601_Negative)
{
	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("-PT1S"));
	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("P-1D"));
	ASSERT_ANY_THROW(TimeDurationUtility::FromIso8601("PT-1S"));
}


TEST(TimeDurationTest, ToIso8601)
{
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromSeconds(3)), "PT3S");
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromMinutes(3)), "PT3M");
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromHours(3)), "PT3H");

	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromDays(3)), "PT72H");
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromDays(31 + 28 + 31)), "PT2160H");
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromDays(3 * 365 + 1)), "PT26304H");

	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromMinutes(3) + TimeDuration::FromSeconds(3)), "PT3M3S");
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromHours(3) + TimeDuration::FromMinutes(3) + TimeDuration::FromSeconds(3)), "PT3H3M3S");
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromHours(3) + TimeDuration::FromSeconds(3)), "PT3H0M3S");
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromHours(3) + TimeDuration::FromMinutes(3)), "PT3H3M");

	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromDays(3 * 365 + 1) +
											 TimeDuration::FromDays(31 + 28 + 31) +
											 TimeDuration::FromDays(3) +
											 TimeDuration::FromHours(3) +
											 TimeDuration::FromMinutes(3) +
											 TimeDuration::FromSeconds(3)), "PT28539H3M3S");
}


TEST(TimeDurationTest, ToIso8601_BaseTime)
{
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromSeconds(3), Time(365 * MillisecondsPerDay)), "PT3S");
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromMinutes(3), Time(365 * MillisecondsPerDay)), "PT3M");
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromHours(3), Time(365 * MillisecondsPerDay)), "PT3H");

	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromDays(3), Time(365 * MillisecondsPerDay)), "P3D");
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromDays(31 + 29), Time(2 * 365 * MillisecondsPerDay)), "P2M");
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromDays(3 * 365), Time(3 * 365 * MillisecondsPerDay)), "P3Y");

	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromDays(3 * 365 + 1) + TimeDuration::FromHours(3), Time(2 * 365 * MillisecondsPerDay)), "P3YT3H");
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromDays(3 * 365 + 1) +
											 TimeDuration::FromDays(31 + 28 + 31) +
											 TimeDuration::FromDays(3) +
											 TimeDuration::FromHours(3) +
											 TimeDuration::FromMinutes(3) +
											 TimeDuration::FromSeconds(3), Time(365 * MillisecondsPerDay)), "P3Y3M3DT3H3M3S");

	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromDays(28), Time(0)), "P28D");
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromDays(28), Time(1 * MillisecondsPerDay)), "P28D");
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromDays(28), Time(2 * MillisecondsPerDay)), "P28D");
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromDays(28), Time(3 * MillisecondsPerDay)), "P28D");
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromDays(28), Time(4 * MillisecondsPerDay)), "P28D");

	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromDays(31), Time(0)), "P1M");
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromDays(31), Time(1 * MillisecondsPerDay)), "P1M");
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromDays(31), Time(2 * MillisecondsPerDay)), "P1M");
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromDays(31), Time(3 * MillisecondsPerDay)), "P1M");
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromDays(31), Time(4 * MillisecondsPerDay)), "P1M");

	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromDays(28), Time(31 * MillisecondsPerDay)), "P1M");
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromDays(28), Time((31 + 1) * MillisecondsPerDay)), "P1M");
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromDays(28), Time((31 + 2) * MillisecondsPerDay)), "P1M");
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromDays(28), Time((31 + 3) * MillisecondsPerDay)), "P1M");
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromDays(28), Time((31 + 4) * MillisecondsPerDay)), "P1M");

	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromDays(31), Time(31 * MillisecondsPerDay)), "P1M3D");
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromDays(31), Time((2 * 365 + 31) * MillisecondsPerDay)), "P1M2D");

	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromSeconds(50), Time(50 * MillisecondsPerSecond)), "PT50S");
}


TEST(TimeDurationTest, ToIso8601_BaseTime_LeapYear)
{
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromDays(365), Time((2 * 365 + 31 + 28) * MillisecondsPerDay)), "P1Y");
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromDays(366), Time((1 * 365 + 31 + 28) * MillisecondsPerDay)), "P1Y");
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromDays(365), Time((1 * 365 + 31 + 27) * MillisecondsPerDay)), "P1Y");
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration::FromDays(365), Time((2 * 365 + 31 + 27) * MillisecondsPerDay)), "P11M30D");
}


TEST(TimeDurationTest, ToIso8601_Zero)
{
	ASSERT_EQ(TimeDurationUtility::ToIso8601(TimeDuration(0)), "PT0S");
}


TEST(TimeDurationTest, ToIso8601_Negative)
{
	ASSERT_ANY_THROW(TimeDurationUtility::ToIso8601(TimeDuration(-1)));
}
