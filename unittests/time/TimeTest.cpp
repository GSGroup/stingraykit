// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <gtest/gtest.h>

#include <stingraykit/string/StringUtils.h>
#include <stingraykit/time/Time.h>

using namespace stingray;

TEST(TimeTest, BrokenDownTime)
{
	Time now = Time::Now();
	{
		BrokenDownTime bdt = now.BreakDown(TimeKind::Local);
		Time reconstructed = Time::FromBrokenDownTime(bdt, TimeKind::Local);
		ASSERT_EQ(now, reconstructed);
	}

	{
		BrokenDownTime bdt = now.BreakDown(TimeKind::Utc);
		Time reconstructed = Time::FromBrokenDownTime(bdt, TimeKind::Utc);
		ASSERT_EQ(now, reconstructed);
	}
}


TEST(TimeTest, TimeZone_FromString)
{
	{
		const auto timeZone = TimeZone::FromString("-05");
		ASSERT_EQ(timeZone.GetMinutesFromUtc(), -300);
	}

	{
		const auto timeZone = TimeZone::FromString("+03");
		ASSERT_EQ(timeZone.GetMinutesFromUtc(), 180);
	}

	{
		const auto timeZone = TimeZone::FromString("0400");
		ASSERT_EQ(timeZone.GetMinutesFromUtc(), 240);
	}

	{
		const auto timeZone = TimeZone::FromString("+0130");
		ASSERT_EQ(timeZone.GetMinutesFromUtc(), 90);
	}

	{
		const auto timeZone = TimeZone::FromString("-02:30");
		ASSERT_EQ(timeZone.GetMinutesFromUtc(), -150);
	}
}


TEST(TimeTest, FromWindowsFileTime)
{
	{
		const auto time = Time::FromWindowsFileTime(0);
		ASSERT_EQ(time.GetMilliseconds(), -11644473600000);
	}

	{
		const auto time = Time::FromWindowsFileTime(864003654321);
		ASSERT_EQ(time.GetMilliseconds(), -11644383545679);

		const auto bdt = time.BreakDown(TimeKind::Utc);

		ASSERT_EQ(bdt.Year,			1601);
		ASSERT_EQ(bdt.Month,		1);
		ASSERT_EQ(bdt.MonthDay,		2);
		ASSERT_EQ(bdt.Hours,		1);
		ASSERT_EQ(bdt.Minutes,		0);
		ASSERT_EQ(bdt.Seconds,		55);
		ASSERT_EQ(bdt.Milliseconds,	-679);
	}
}


TEST(TimeTest, MjdDate)
{
#if 0
		{
			BrokenDownTime bdt = Time::MjdToEpoch(0).BreakDown(TimeKind::Utc);
			ASSERT_EQ(bdt.Year			, 1858);
			ASSERT_EQ(bdt.Month			, 11);
			ASSERT_EQ(bdt.MonthDay		, 17);
			ASSERT_EQ(bdt.Hours			, 0);
			ASSERT_EQ(bdt.Minutes		, 0);
			ASSERT_EQ(bdt.Seconds		, 0);
			ASSERT_EQ(bdt.Milliseconds	, 0);
		}
#endif
		{
			BrokenDownTime bdt = Time::MjdToEpoch(57023).BreakDown(TimeKind::Utc); //2015-01-01
			ASSERT_EQ(bdt.Year			, 2015);
			ASSERT_EQ(bdt.Month			, 1);
			ASSERT_EQ(bdt.MonthDay		, 1);
			ASSERT_EQ(bdt.Hours			, 0);
			ASSERT_EQ(bdt.Minutes		, 0);
			ASSERT_EQ(bdt.Seconds		, 0);
			ASSERT_EQ(bdt.Milliseconds	, 0);
		}
		{
			BrokenDownTime bdt = Time::MjdToEpoch(60676).BreakDown(TimeKind::Utc); //2025-01-01
			ASSERT_EQ(bdt.Year			, 2025);
			ASSERT_EQ(bdt.Month			, 1);
			ASSERT_EQ(bdt.MonthDay		, 1);
			ASSERT_EQ(bdt.Hours			, 0);
			ASSERT_EQ(bdt.Minutes		, 0);
			ASSERT_EQ(bdt.Seconds		, 0);
			ASSERT_EQ(bdt.Milliseconds	, 0);
		}
}


TEST(TimeTest, EpochDate)
{
	{
		BrokenDownTime bdt = Time::FromTimeT(1735689600).BreakDown(TimeKind::Utc); //2025-01-01
		ASSERT_EQ(bdt.Year			, 2025);
		ASSERT_EQ(bdt.Month			, 1);
		ASSERT_EQ(bdt.MonthDay		, 1);
		ASSERT_EQ(bdt.Hours			, 0);
		ASSERT_EQ(bdt.Minutes		, 0);
		ASSERT_EQ(bdt.Seconds		, 0);
		ASSERT_EQ(bdt.Milliseconds	, 0);
	}
	{
		BrokenDownTime bdt = Time::FromTimeT(1420070400).BreakDown(TimeKind::Utc); //2015-01-01
		ASSERT_EQ(bdt.Year			, 2015);
		ASSERT_EQ(bdt.Month			, 1);
		ASSERT_EQ(bdt.MonthDay		, 1);
		ASSERT_EQ(bdt.Hours			, 0);
		ASSERT_EQ(bdt.Minutes		, 0);
		ASSERT_EQ(bdt.Seconds		, 0);
		ASSERT_EQ(bdt.Milliseconds	, 0);
	}
}


TEST(TimeTest, DISABLED_ToIso8601_Zeroes)
{
	const std::string sample = "0000-00-00T00:00:00Z";
	const Time testee = TimeUtility::FromIso8601(sample);
	const BrokenDownTime brokenDown = testee.BreakDown(TimeKind::Utc);
	ASSERT_EQ(brokenDown.Year, 0);
	ASSERT_EQ(brokenDown.Month, 0);
	ASSERT_EQ(brokenDown.MonthDay, 0);
	ASSERT_EQ(brokenDown.Hours, 0);
	ASSERT_EQ(brokenDown.Minutes, 0);
	ASSERT_EQ(brokenDown.Seconds, 0);
	ASSERT_EQ(brokenDown.Milliseconds, 0);
}


TEST(TimeTest, ToIso8601_Common)
{
	const std::string sample = "2000-09-21T12:45:05Z";
	const Time testee = TimeUtility::FromIso8601(sample);
	const BrokenDownTime brokenDown = testee.BreakDown(TimeKind::Utc);
	ASSERT_EQ(brokenDown.Year, 2000);
	ASSERT_EQ(brokenDown.Month, 9);
	ASSERT_EQ(brokenDown.MonthDay, 21);
	ASSERT_EQ(brokenDown.Hours, 12);
	ASSERT_EQ(brokenDown.Minutes, 45);
	ASSERT_EQ(brokenDown.Seconds, 5);
	ASSERT_EQ(brokenDown.Milliseconds, 0);
}


TEST(TimeTest, ToIso8601_Lowercase)
{
	const std::string sample = "2000-09-21t12:45:05z";
	const Time testee = TimeUtility::FromIso8601(sample);
	const BrokenDownTime brokenDown = testee.BreakDown(TimeKind::Utc);
	ASSERT_EQ(brokenDown.Year, 2000);
	ASSERT_EQ(brokenDown.Month, 9);
	ASSERT_EQ(brokenDown.MonthDay, 21);
	ASSERT_EQ(brokenDown.Hours, 12);
	ASSERT_EQ(brokenDown.Minutes, 45);
	ASSERT_EQ(brokenDown.Seconds, 5);
	ASSERT_EQ(brokenDown.Milliseconds, 0);
}


TEST(TimeTest, ToIso8601_SecondFraction)
{
	const std::string sample = "2000-09-21T12:45:05.0Z";
	const Time testee = TimeUtility::FromIso8601(sample);
	const BrokenDownTime brokenDown = testee.BreakDown(TimeKind::Utc);
	ASSERT_EQ(brokenDown.Year, 2000);
	ASSERT_EQ(brokenDown.Month, 9);
	ASSERT_EQ(brokenDown.MonthDay, 21);
	ASSERT_EQ(brokenDown.Hours, 12);
	ASSERT_EQ(brokenDown.Minutes, 45);
	ASSERT_EQ(brokenDown.Seconds, 5);
	ASSERT_NEAR(brokenDown.Milliseconds, 0, 5);
}


TEST(TimeTest, Iso8601_SecondFractionDecade)
{
	const std::string sample = "2000-09-21T12:45:05.8Z";
	const Time testee = TimeUtility::FromIso8601(sample);
	BrokenDownTime brokenDown = testee.BreakDown(TimeKind::Utc);
	ASSERT_EQ(brokenDown.Year, 2000);
	ASSERT_EQ(brokenDown.Month, 9);
	ASSERT_EQ(brokenDown.MonthDay, 21);
	ASSERT_EQ(brokenDown.Hours, 12);
	ASSERT_EQ(brokenDown.Minutes, 45);
	ASSERT_EQ(brokenDown.Seconds, 5);
	ASSERT_NEAR(brokenDown.Milliseconds, 800, 5);
}


TEST(TimeTest, Iso8601_ZeroOffset)
{
	const std::string sample = "2000-09-21T12:45:05+00:00";
	const Time testee = TimeUtility::FromIso8601(sample);
	const BrokenDownTime brokenDown = testee.BreakDown(TimeKind::Utc);
	ASSERT_EQ(brokenDown.Year, 2000);
	ASSERT_EQ(brokenDown.Month, 9);
	ASSERT_EQ(brokenDown.MonthDay, 21);
	ASSERT_EQ(brokenDown.Hours, 12);
	ASSERT_EQ(brokenDown.Minutes, 45);
	ASSERT_EQ(brokenDown.Seconds, 5);
	ASSERT_EQ(brokenDown.Milliseconds, 0);
}


TEST(TimeTest, ToIso8601_PositiveOffset)
{
	const std::string sample = "2000-09-21T12:45:05+13:53";
	const Time testee = TimeUtility::FromIso8601(sample);
	const BrokenDownTime brokenDown = testee.BreakDown(TimeKind::Utc);
	ASSERT_EQ(brokenDown.Year, 2000);
	ASSERT_EQ(brokenDown.Month, 9);
	ASSERT_EQ(brokenDown.MonthDay, 20);
	ASSERT_EQ(brokenDown.Hours, 22);
	ASSERT_EQ(brokenDown.Minutes, 52);
	ASSERT_EQ(brokenDown.Seconds, 5);
	ASSERT_EQ(brokenDown.Milliseconds, 0);
}


TEST(TimeTest, ToIso8601_NegativeOffset)
{
	const std::string sample = "2000-09-21T12:45:05-13:53";
	const Time testee = TimeUtility::FromIso8601(sample);
	const BrokenDownTime brokenDown = testee.BreakDown(TimeKind::Utc);
	ASSERT_EQ(brokenDown.Year, 2000);
	ASSERT_EQ(brokenDown.Month, 9);
	ASSERT_EQ(brokenDown.MonthDay, 22);
	ASSERT_EQ(brokenDown.Hours, 2);
	ASSERT_EQ(brokenDown.Minutes, 38);
	ASSERT_EQ(brokenDown.Seconds, 5);
	ASSERT_EQ(brokenDown.Milliseconds, 0);
}


TEST(TimeTest, ToIso8601_DateOnly)
{
	const std::string sample = "2000-09-21";
	const Time testee = TimeUtility::FromIso8601(sample);
	const BrokenDownTime brokenDown = testee.BreakDown(TimeKind::Utc);
	ASSERT_EQ(brokenDown.Year, 2000);
	ASSERT_EQ(brokenDown.Month, 9);
	ASSERT_EQ(brokenDown.MonthDay, 21);
	ASSERT_EQ(brokenDown.Hours, 0);
	ASSERT_EQ(brokenDown.Minutes, 0);
	ASSERT_EQ(brokenDown.Seconds, 0);
	ASSERT_EQ(brokenDown.Milliseconds, 0);
}


TEST(TimeTest, ToIso8601_NoT_Failure)
{
	const std::string sample = "2000-09-21.12:45:05Z";
	ASSERT_ANY_THROW(TimeUtility::FromIso8601(sample));
}
