// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/time/Time.h>

#include <stingraykit/serialization/Serialization.h>
#include <stingraykit/string/regex.h>
#include <stingraykit/string/StringFormat.h>
#include <stingraykit/string/StringParse.h>
#include <stingraykit/time/TimeEngine.h>
#include <stingraykit/math.h>

#include <stdio.h>

namespace stingray
{

	namespace
	{

		const int SecondsPerMinute			= 60;
		const int MinutesPerHour			= 60;
		const int HoursPerDay				= 24;
		const int DaysPerWeek				= 7;
		const int MillisecondsPerMinute		= SecondsPerMinute * 1000;
		const int SecondsPerHour			= MinutesPerHour * SecondsPerMinute;
		const int SecondsPerDay				= HoursPerDay * SecondsPerHour;
		const int DaysSinceMjd				= 40587;

		const s64 SecondsBetweenNtpAndUnixEpochs = 2208988800ll;

		const u32 WindowsTicksPerSecond = 10000000;
		const u64 SecondsBetweenWindowsAndUnixEpochs = 11644473600;

		u8 BcdValue(u8 byte)
		{ return ((byte >> 4) & 0x0f) * 10 + (byte & 0x0f); }

		u8 BcdEncode(u8 value)
		{ return ((value / 10) << 4) + value % 10; }

		int GetMaxDaysInYear(int year)
		{ return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0) ? 366 : 365; }

	}


	void TimeDuration::Serialize(ObjectOStream& ar) const
	{ ar.Serialize("us", _microseconds); }


	void TimeDuration::Deserialize(ObjectIStream& ar)
	{
		optional<s64> microseconds;
		ar.Deserialize("us", microseconds);
		if (microseconds)
		{
			_microseconds = *microseconds;
			return;
		}

		s64 milliseconds = 0;
		ar.Deserialize("ms", milliseconds);
		_microseconds = milliseconds * 1000;
	}


	std::string TimeDuration::ToString(const std::string& format) const
	{
		std::string result = format.empty() ? "hh:mm:ss.lll" : format;
		if (GetMilliseconds() < 0)
			result.insert(0, "-");

		const std::string hoursPlaceholder = "hh";
		const bool hasHours = std::search(result.begin(), result.end(), hoursPlaceholder.begin(), hoursPlaceholder.end()) != result.end();

		const s64 absMs = Absolute().GetMilliseconds();
		const s64 hours = hasHours ? absMs / Hour().GetMilliseconds() : 0;
		const s64 minutes = (absMs - hours * Hour().GetMilliseconds()) / Minute().GetMilliseconds();
		const s64 seconds = absMs % Minute().GetMilliseconds() / Second().GetMilliseconds();
		const s64 milliseconds = absMs % Second().GetMilliseconds();

		if (hasHours)
			ReplaceAll(result, "hh", StringFormat("%1$2%", hours));
		else
			ReplaceAll(result, "hh", "");

		ReplaceAll(result, "mm", StringFormat("%1$2%", minutes));
		ReplaceAll(result, "ss", StringFormat("%1$2%", seconds));
		ReplaceAll(result, "lll", StringFormat("%1$3%", milliseconds));
		return result;
	}


	TimeDuration TimeDuration::FromString(const std::string& str)
	{
		int value = 0;
		char units = 0;
		const int components = sscanf(str.c_str(), "%d%c", &value, &units);
		STINGRAYKIT_CHECK(components > 0, FormatException(str));

		switch (units)
		{
		case 'H':
		case 'h':
			return FromHours(value);
		case 'M':
		case 'm':
			return FromMinutes(value);
		case 'S':
		case 's':
			return FromSeconds(value);
		case 0:
			return TimeDuration(value);
		}

		STINGRAYKIT_THROW(FormatException(str));
	}


	TimeZone::TimeZone(s16 minutes)
		: _minutesFromUtc(minutes)
	{ STINGRAYKIT_CHECK(minutes >= -12 * MinutesPerHour && minutes <= 14 * MinutesPerHour, ArgumentException("minutes", minutes)); }


	TimeZone TimeZone::Current()
	{ return TimeZone(TimeEngine::GetMinutesFromUtc()); }


	std::string TimeZone::ToString() const
	{
		const std::string sign = _minutesFromUtc > 0 ? "+" : _minutesFromUtc < 0 ? "-" : "";
		return StringBuilder() % sign % (Abs(_minutesFromUtc) / MinutesPerHour) % ":" % (Abs(_minutesFromUtc) % MinutesPerHour);
	}


	TimeZone TimeZone::FromString(const std::string& str)
	{
		STINGRAYKIT_CHECK(!str.empty(), FormatException(str));

		const optional<char> sign = str[0] == '+' || str[0] == '-' ? make_optional_value(str[0]) : null;
		const auto delimiterPos = str.find(':');

		int hours = 0;
		int minutes = 0;
		if (delimiterPos != std::string::npos)
			STINGRAYKIT_CHECK(StringParse(sign ? str.substr(1) : str, "%1%:%2%", hours, minutes), FormatException(str));
		else
		{
			const size_t baseIndex = sign ? 1 : 0;
			const auto strSize = str.size();
			STINGRAYKIT_CHECK(strSize == baseIndex + 2 || strSize == baseIndex + 4, FormatException(str));

			hours = stingray::FromString<int>(str.substr(baseIndex, 2));
			minutes = strSize > baseIndex + 2 ? stingray::FromString<int>(str.substr(baseIndex + 2, 2)) : 0;
		}

		const int value = hours * MinutesPerHour + minutes;
		return TimeZone(!sign || *sign == '+' ? value : -value);
	}


	void TimeZone::Serialize(ObjectOStream& ar) const
	{ ar.Serialize("offset", _minutesFromUtc); }


	void TimeZone::Deserialize(ObjectIStream& ar)
	{ ar.Deserialize("offset", _minutesFromUtc); }


	Time::Time(s64 milliseconds)
		: _milliseconds(milliseconds)
	{ }


	Time Time::Now()
	{ return Time(TimeEngine::GetMillisecondsSinceEpoch()); }


	BrokenDownTime Time::BreakDown(TimeKind kind) const
	{
		const s64 offset = kind == TimeKind::Utc ? 0 : MillisecondsPerMinute * TimeEngine::GetMinutesFromUtc();
		return TimeEngine::BrokenDownFromMilliseconds(_milliseconds + offset);
	}


	Time Time::FromBrokenDownTime(const BrokenDownTime& bdt, TimeKind kind)
	{
		const s64 offset = kind == TimeKind::Utc ? 0 : MillisecondsPerMinute * TimeEngine::GetMinutesFromUtc();
		return Time(TimeEngine::MillisecondsFromBrokenDown(bdt) - offset);
	}


	std::string Time::ToString(const std::string& format, TimeKind kind) const
	{ return BreakDown(kind).ToString(format); }


	Time Time::FromString(const std::string& str, TimeKind kind)
	{
		if (str == "now")
			return Time::Now();

		if (str.size() > 4 && str.substr(0, 4) == "now+")
			return Time::Now() + TimeDuration::FromString(str.substr(4));

		s16 year = 0;
		s16 month = 0;
		s16 day = 0;
		s16 hour = 0;
		s16 minute = 0;
		s16 second = 0;
		char utcSign = 0;
		s16 utcHour = 0;
		s16 utcMinute = 0;

		bool haveDate = false;
		bool haveTime = false;
		bool haveSeconds = false;
		bool haveUtcSign = false;
		bool haveUtcHours = false;
		bool haveUtcMinutes = false;

		int components = sscanf(str.c_str(), "%hd.%hd.%hd %hd:%hd:%hd", &day, &month, &year, &hour, &minute, &second);
		if (components >= 3)
		{
			haveDate = true;
			if (components >= 5)
				haveTime = true;
			if (components >= 6)
				haveSeconds = true;
		}
		else
		{
			components = sscanf(str.c_str(), "%hd/%hd/%hd %hd:%hd:%hd", &day, &month, &year, &hour, &minute, &second);
			if (components >= 3)
			{
				haveDate = true;
				if (components >= 5)
					haveTime = true;
				if (components >= 6)
					haveSeconds = true;
			}
			else
			{
				components = sscanf(str.c_str(), "%hd:%hd:%hd", &hour, &minute, &second);
				if (components >= 2)
				{
					haveTime = true;
					if (components >= 3)
						haveSeconds = true;
				}
				else
				{
					components = sscanf(str.c_str(), "%hd-%hd-%hdT%hd:%hd:%hd%c%hd:%hd", &year, &month, &day, &hour, &minute, &second, &utcSign, &utcHour, &utcMinute);
					STINGRAYKIT_CHECK(components >= 3, FormatException(str));

					haveDate = true;
					if (components >= 5)
						haveTime = true;
					if (components >= 6)
						haveSeconds = true;
					if (components >= 7)
						haveUtcSign = true;
					if (components >= 8)
						haveUtcHours = true;
					if (components >= 9)
						haveUtcMinutes = true;
				}
			}
		}
		STINGRAYKIT_CHECK(haveDate || haveTime, FormatException(str));
		STINGRAYKIT_CHECK(!(!haveTime && haveSeconds), FormatException(str));
		STINGRAYKIT_CHECK(!haveUtcSign || ((utcSign == 'Z' && !haveUtcHours && !haveUtcMinutes) || ((utcSign == '+' || utcSign == '-') && haveUtcHours && !(!haveUtcHours && haveUtcMinutes))), FormatException(str));

		BrokenDownTime bdt;
		if (haveDate)
		{
			bdt.MonthDay		= day;
			bdt.Month			= month;
			bdt.Year			= year > 100 ? year : year > 30 ? 1900 + year : 2000 + year;
		}
		else
		{
			bdt					= Time::Now().BreakDown();
			bdt.Seconds			= 0;
			bdt.Milliseconds	= 0;
		}

		if (haveTime)
		{
			bdt.Hours			= hour;
			bdt.Minutes			= minute;
		}

		if (haveSeconds)
			bdt.Seconds			= second;

		if (haveUtcSign)
		{
			if (utcSign == '+' || utcSign == '-')
			{
				const s16 minutesFromUtc = (haveUtcHours ? (utcHour * MinutesPerHour) : 0) + (haveUtcMinutes ? utcMinute : 0);
				if (utcSign == '-')
					bdt.Minutes += minutesFromUtc;
				else
					bdt.Minutes -= minutesFromUtc;
			}
			else
				STINGRAYKIT_CHECK(utcSign == 'Z', FormatException(str));
		}

		return FromBrokenDownTime(bdt, haveUtcSign ? TimeKind(TimeKind::Utc) : kind);
	}


	u64 Time::ToNtpTimestamp() const
	{ return GetMilliseconds() / 1000 + SecondsBetweenNtpAndUnixEpochs; }


	Time Time::FromNtpTimestamp(u64 timestamp)
	{ return Time((timestamp - SecondsBetweenNtpAndUnixEpochs) * 1000); }


	Time Time::FromWindowsFileTime(u64 windowsTicks)
	{ return Time((windowsTicks / WindowsTicksPerSecond - SecondsBetweenWindowsAndUnixEpochs) * 1000 + windowsTicks % WindowsTicksPerSecond); }


	Time Time::MJDtoEpoch(int mjd, u32 bcdTime)
	{ return Time(s64(mjd - DaysSinceMjd) * SecondsPerDay * 1000) + BCDDurationToTimeDuration(bcdTime); }


	TimeDuration Time::BCDDurationToTimeDuration(u32 bcdTime)
	{
		return TimeDuration(s64(1000) * (SecondsPerHour * BcdValue((bcdTime >> 16) & 0xff) +
				SecondsPerMinute * BcdValue((bcdTime >> 8) & 0xff) +
				BcdValue(bcdTime & 0xff)));
	}


	int Time::GetMJD() const
	{ return DaysSinceMjd + _milliseconds / (1000 * SecondsPerDay); }


	u32 Time::GetBCDTime(TimeKind kind) const
	{
		const BrokenDownTime bdt(BreakDown(kind));
		return (BcdEncode(bdt.Hours) << 16) + (BcdEncode(bdt.Minutes) << 8) + BcdEncode(bdt.Seconds);
	}


	int Time::DaysTo(const Time& endTime) const
	{ return DaysTo(endTime.BreakDown()); }


	int Time::DaysTo(const BrokenDownTime& endTime) const
	{ return (Time::FromBrokenDownTime(endTime.GetDayStart()) - Time::FromBrokenDownTime(BreakDown().GetDayStart())) / TimeDuration::Day(); }


	void Time::Serialize(ObjectOStream& ar) const
	{ ar.Serialize("ms", _milliseconds); }


	void Time::Deserialize(ObjectIStream& ar)
	{ ar.Deserialize("ms", _milliseconds); }


	class TimeUtility::FromIso8601Impl
	{
		struct ParseResult
		{
			s16					Year;
			s16					Month;
			s16					Day;
			s16					Hours;
			s16					Minutes;
			double				Seconds;
			TimeDuration		Offset;

			ParseResult()
				: Year(0), Month(0), Day(0), Hours(0), Minutes(0), Seconds(0), Offset()
			{ }
		};

	public:
		Time operator () (const std::string& str) const
		{
			const std::string uppercase = ToUpper(str); // Rfc3339 5.6 NOTE

			optional<ParseResult> result = TryFromDateTime(uppercase);

			if (!result)
				result = TryFromDateTimeWithOffset(uppercase);

			if (!result)
				result = TryFromDate(uppercase);

			STINGRAYKIT_CHECK(result, FormatException(str));

			const s16 milliseconds = (result->Seconds - double(s16(result->Seconds))) * 1000.;
			const BrokenDownTime brokenDown(milliseconds, (s16)result->Seconds, result->Minutes, result->Hours, 0, result->Day, result->Month, 0, result->Year);
			const Time local = Time::FromBrokenDownTime(brokenDown, TimeKind::Utc);

			return local - result->Offset;
		}

	private:
		static optional<ParseResult> TryFromDateTime(const std::string& str)
		{
			ParseResult result;

			if (!StringParse(str, "%1%-%2%-%3%T%4%:%5%:%6%Z", result.Year, result.Month, result.Day, result.Hours, result.Minutes, result.Seconds)
					&& !StringParse(str, "%1%-%2%-%3%T%4%:%5%:%6%", result.Year, result.Month, result.Day, result.Hours, result.Minutes, result.Seconds))
				return null;

			return result;
		}

		static optional<ParseResult> TryFromDateTimeWithOffset(const std::string& str)
		{
			ParseResult result;

			const size_t SignPosFromEnd = 5;
			if (str.size() <= SignPosFromEnd)
				return null;

			const char sign = str[str.size() - 1 - SignPosFromEnd];
			const s8 multiplier = sign == '+' ? 1 : sign == '-' ? -1 : 0;
			if (multiplier == 0)
				return null;

			s16 offsetHours = 0;
			s16 offsetMinutes = 0;
			const bool success = StringParse(
					str,
					StringBuilder() % "%1%-%2%-%3%T%4%:%5%:%6%" % sign % "%7%:%8%",
					result.Year,
					result.Month,
					result.Day,
					result.Hours,
					result.Minutes,
					result.Seconds,
					offsetHours,
					offsetMinutes);

			if (!success)
				return null;

			result.Offset = TimeDuration::FromHours(offsetHours) + TimeDuration::FromMinutes(offsetMinutes);
			result.Offset *= multiplier;

			return result;
		}

		static optional<ParseResult> TryFromDate(const std::string& str)
		{
			ParseResult result;
			if (!StringParse(str, "%1%-%2%-%3%", result.Year, result.Month, result.Day))
				return null;

			return result;
		}
	};


	std::string TimeUtility::ToIso8601(const Time& time)
	{ return time.BreakDown(TimeKind::Utc).ToString("YYYY-MM-ddThh:mm:ss.lllZ"); }


	Time TimeUtility::FromIso8601(const std::string& str)
	{ return FromIso8601Impl()(str); }


	class TimeDurationUtility::FromIso8601Impl
	{
		struct ParseResult
		{
			s16					Years;
			s16					Months;
			s16					Weeks;
			s16					Days;
			s16					Hours;
			s16					Minutes;
			s16					Seconds;

			ParseResult()
				: Years(0), Months(0), Weeks(0), Days(0), Hours(0), Minutes(0), Seconds(0)
			{ }

			TimeDuration ToTimeDuration(Time base, TimeKind kind) const
			{
				if (Years == 0 && Months == 0)
					return TimeDuration::FromDays(Weeks * DaysPerWeek + Days) + TimeDuration::FromHours(Hours) + TimeDuration::FromMinutes(Minutes) + TimeDuration::FromSeconds(Seconds);

				const BrokenDownTime bdt = base.BreakDown(kind);
				return Time::FromBrokenDownTime(
						BrokenDownTime(
								bdt.Milliseconds,
								bdt.Seconds + Seconds,
								bdt.Minutes + Minutes,
								bdt.Hours + Hours,
								bdt.WeekDay,
								bdt.MonthDay + Weeks * DaysPerWeek + Days,
								bdt.Month + Months,
								bdt.YearDay,
								bdt.Year + Years),
						kind) - base;
			}
		};

	public:
		TimeDuration operator () (const std::string& str, Time base) const
		{
			const std::string uppercase = ToUpper(str);

			ParseResult result;

			smatch m;
			STINGRAYKIT_CHECK(regex_search(uppercase, m, regex("^P([0-9WYMDTHS]{2,})$")), FormatException(str));
			STINGRAYKIT_CHECK(TryFromDateTime(m[1], result) || TryFromWeek(m[1], result), FormatException(str));

			return result.ToTimeDuration(base, TimeKind::Utc);
		}

	private:
		static bool TryFromDateTime(const std::string& str, ParseResult& result)
		{
			smatch m;
			if (regex_search(str, m, regex("^(.*)T(.*)$")))
				return !m[2].empty() && TryFromTime(m[2], result) && (m[1].empty() || TryFromDate(m[1], result));
			else
				return TryFromDate(str, result);
		}

		static bool TryFromWeek(const std::string& str, ParseResult& result)
		{ return StringParse(str, "%1%W", result.Weeks); }

		static bool TryFromTime(const std::string& str, ParseResult& result)
		{
			std::string toParse = str;
			std::string remaining;

			if (Contains(toParse, "H"))
			{
				if (EndsWith(toParse, "H"))
					return StringParse(toParse, "%1%H", result.Hours);

				if (!StringParse(toParse, "%1%H%2%", result.Hours, remaining))
					return false;

				toParse = remaining;
				remaining.clear();
			}

			if (Contains(toParse, "M"))
			{
				if (EndsWith(toParse, "M"))
					return StringParse(toParse, "%1%M", result.Minutes);

				if (!StringParse(toParse, "%1%M%2%", result.Minutes, remaining))
					return false;

				toParse = remaining;
				remaining.clear();
			}

			return StringParse(toParse, "%1%S", result.Seconds);
		}

		static bool TryFromDate(const std::string& str, ParseResult& result)
		{
			std::string toParse = str;
			std::string remaining;

			if (Contains(toParse, "Y"))
			{
				if (EndsWith(toParse, "Y"))
					return StringParse(toParse, "%1%Y", result.Years);

				if (!StringParse(toParse, "%1%Y%2%", result.Years, remaining))
					return false;

				toParse = remaining;
				remaining.clear();
			}

			if (Contains(toParse, "M"))
			{
				if (EndsWith(toParse, "M"))
					return StringParse(toParse, "%1%M", result.Months);

				if (!StringParse(toParse, "%1%M%2%", result.Months, remaining))
					return false;

				toParse = remaining;
				remaining.clear();
			}

			return StringParse(toParse, "%1%D", result.Days);
		}
	};


	std::string TimeDurationUtility::ToIso8601(TimeDuration duration, const optional<Time>& base)
	{
		STINGRAYKIT_CHECK(duration >= TimeDuration(), ArgumentException("duration", duration));

		s16 Years = 0;
		s16 Months = 0;
		s16 Days = 0;
		s16 Hours = 0;
		s16 Minutes = 0;
		s16 Seconds = 0;

		if (!base)
		{
			Years	= 0;
			Months	= 0;
			Days	= 0;
			Hours	= duration.GetHours();
			Minutes	= duration.GetMinutes() % MinutesPerHour;
			Seconds	= duration.GetSeconds() % SecondsPerMinute;
		}
		else
		{
			Years	= 0;
			Months	= 0;
			Days	= duration.GetDays();
			Hours	= duration.GetHours() % HoursPerDay;
			Minutes	= duration.GetMinutes() % MinutesPerHour;
			Seconds	= duration.GetSeconds() % SecondsPerMinute;

			BrokenDownTime bdt = base->BreakDown(TimeKind::Utc);
			int size = 0;

			if (bdt.Month > 2 || (bdt.Month == 2 && bdt.MonthDay == 29))
				bdt.Year++;

			while ((size = GetMaxDaysInYear(bdt.Year)) <= Days)
			{
				Days -= size;
				bdt.Year++;
				Years++;
			}

			if (bdt.Month > 2 || (bdt.Month == 2 && bdt.MonthDay == 29))
				bdt.Year--;

			while ((size = bdt.GetMaxDaysInMonth()) <= Days)
			{
				Days -= size;
				bdt.Month++;

				if (bdt.Month > 12)
				{
					bdt.Month = 1;
					bdt.Year++;
				}

				Months++;
			}
		}

		const bool hasDate = Years != 0 || Months != 0 || Days != 0;
		const bool hasTime = Hours != 0 || Minutes != 0 || Seconds != 0;

		if (!hasDate && !hasTime)
			return "PT0S";

		StringBuilder str;
		str % "P";

		if (hasDate)
		{
			if (Years != 0)
				str % Years % "Y";

			if (Months != 0 || (Years != 0 && Days != 0))
				str % Months % "M";

			if (Days != 0)
				str % Days % "D";
		}

		if (hasTime)
		{
			str % "T";
			if (Hours != 0)
				str % Hours % "H";

			if (Minutes != 0 || (Hours != 0 && Seconds != 0))
				str % Minutes % "M";

			if (Seconds != 0)
				str % Seconds % "S";
		}

		return str;
	}


	TimeDuration TimeDurationUtility::FromIso8601(const std::string& str, Time base)
	{ return FromIso8601Impl()(str, base); };

}
