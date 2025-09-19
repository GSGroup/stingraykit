// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/time/Time.h>

#include <stingraykit/serialization/Serialization.h>
#include <stingraykit/string/RegexUtils.h>
#include <stingraykit/string/StringParse.h>
#include <stingraykit/string/StringUtils.h>
#include <stingraykit/time/TimeEngine.h>
#include <stingraykit/math.h>

#include <regex>

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

		const s64 MicrosecondsDivisors[] = { 1000000, 100000, 10000, 1000, 100, 10, 1 };

		const s64 SecondsBetweenNtpAndUnixEpochs = 2208988800ll;

		const u32 WindowsTicksPerSecond = 10000000;
		const u64 SecondsBetweenWindowsAndUnixEpochs = 11644473600;

		u8 BcdValue(u8 byte)
		{ return ((byte >> 4) & 0x0f) * 10 + (byte & 0x0f); }

		u8 BcdEncode(u8 value)
		{ return ((value / 10) << 4) + value % 10; }

		int GetMaxDaysInYear(int year)
		{ return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0) ? 366 : 365; }

		TimeDuration ParseSeconds(string_view secondsStr)
		{
			u64 seconds = 0;
			string_view fractionStr;

			STINGRAYKIT_CHECK(
					StringParse(secondsStr, "%1%.%2%", seconds, fractionStr)
							|| StringParse(secondsStr, "%1%", seconds),
					FormatException(secondsStr));

			TimeDuration result = TimeDuration::FromSeconds(seconds);

			if (!fractionStr.empty())
			{
				const FractionInfo fraction = ParseDecimalFraction(fractionStr, 6);

				result += TimeDuration::FromMicroseconds(fraction.Fraction);
				if (fraction.IsOverflow)
					result += TimeDuration::Second();
			}

			return result;
		}

	}


	TimeDuration TimeDuration::Absolute() const
	{
		STINGRAYKIT_CHECK(_microseconds > std::numeric_limits<s64>::min(), IntegerOverflowException(std::numeric_limits<s64>::max(), 1));
		return TimeDuration(Abs(_microseconds), Dummy());
	}


	TimeDuration TimeDuration::RoundToMilliseconds() const
	{ return TimeDuration::FromMilliseconds(GetMilliseconds() + (Abs(_microseconds % 1000) < 500 ? 0 : _microseconds < 0 ? -1 : 1)); }


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


	std::string TimeDuration::ToString(string_view format) const
	{
		if (format.empty())
		{
			StringBuilder result;
			if (GetMilliseconds() < 0)
				result % '-';

			const s64 absMs = Abs(GetMilliseconds());
			const s64 hours = absMs / Hour().GetMilliseconds();
			const s64 minutes = absMs % Hour().GetMilliseconds() / Minute().GetMilliseconds();
			const s64 seconds = absMs % Minute().GetMilliseconds() / Second().GetMilliseconds();
			const s64 milliseconds = absMs % Second().GetMilliseconds();

			result % RightJustify(stingray::ToString(hours), 2, '0') % ':';
			result % RightJustify(stingray::ToString(minutes), 2, '0') % ':';
			result % RightJustify(stingray::ToString(seconds), 2, '0') % '.';
			result % RightJustify(stingray::ToString(milliseconds), 3, '0');

			return result;
		}

		StringBuilder result;

		bool hasHours = false;
		bool hasMinutes = false;
		bool hasSeconds = false;
		size_t maxPrecision = 0;

		for (size_t pos = 0; pos < format.size(); ++pos)
		{
			const char c = format[pos];

			if (c == 'h')
				hasHours = true;
			else if (c == 'm')
				hasMinutes = true;
			else if (c == 's')
				hasSeconds = true;
			else if (c == 'l')
			{
				const size_t lastPos = format.find_first_not_of('l', pos + 1);
				const size_t precision = (lastPos == string_view::npos ? format.size() : lastPos) - pos;

				maxPrecision = std::max(maxPrecision, precision);
				pos += precision - 1;
			}
		}

		const bool negative = GetMicroseconds() / ArrayGet(MicrosecondsDivisors, std::min(maxPrecision, ArraySize(MicrosecondsDivisors) - 1)) < 0;

		const s64 hours = hasHours ? Abs(_microseconds / Hour().GetMicroseconds()) : 0;
		const s64 minutes = Abs(_microseconds < 0 ? _microseconds + hours * Hour().GetMicroseconds() : _microseconds - hours * Hour().GetMicroseconds()) / Minute().GetMicroseconds();
		const s64 seconds = Abs(_microseconds % Minute().GetMicroseconds()) / Second().GetMicroseconds();
		const s64 microseconds = Abs(_microseconds % Second().GetMicroseconds());

		for (size_t pos = 0; pos < format.size(); ++pos)
		{
			const char c = format[pos];

			switch (c)
			{
			case '\\':
				STINGRAYKIT_CHECK(format.size() - pos > 1, FormatException(format));
				result % format[pos + 1];
				++pos;
				break;

			case 'h':
				if (negative)
					result % '-';

				if (format.size() - pos > 1 && format[pos + 1] == 'h')
				{
					result % RightJustify(stingray::ToString(hours), 2, '0');
					++pos;
				}
				else
					result % hours;
				break;

			case 'm':
				if (!hasHours && negative)
					result % '-';

				if (format.size() - pos > 1 && format[pos + 1] == 'm')
				{
					result % RightJustify(stingray::ToString(minutes), 2, '0');
					++pos;
				}
				else
					result % minutes;
				break;

			case 's':
				if (!hasHours && !hasMinutes && negative)
					result % '-';

				if (format.size() - pos > 1 && format[pos + 1] == 's')
				{
					result % RightJustify(stingray::ToString(seconds), 2, '0');
					++pos;
				}
				else
					result % seconds;
				break;

			case 'l':
			{
				if (!hasHours && !hasMinutes && !hasSeconds && negative)
					result % '-';

				const size_t lastPos = format.find_first_not_of('l', pos + 1);
				const size_t precision = (lastPos == string_view::npos ? format.size() : lastPos) - pos;
				const size_t croppedPrecision = std::min(precision, ArraySize(MicrosecondsDivisors) - 1);

				result % RightJustify(stingray::ToString(microseconds / ArrayGet(MicrosecondsDivisors, croppedPrecision)), croppedPrecision, '0');
				pos += precision - 1;
				break;
			}

			default:
				result % c;
				break;
			}
		}

		return result;
	}


	TimeDuration TimeDuration::FromString(string_view str)
	{
		int value = 0;

		if (StringParse(str, "%1%h", value) || StringParse(str, "%1%H", value))
			return FromHours(value);

		if (StringParse(str, "%1%m", value) || StringParse(str, "%1%M", value))
			return FromMinutes(value);

		if (StringParse(str, "%1%s", value) || StringParse(str, "%1%S", value))
			return FromSeconds(value);

		STINGRAYKIT_CHECK(StringParse(str, "%1%", value), FormatException(str));
		return TimeDuration::FromMilliseconds(value);
	}


	TimeDuration TimeDuration::FromBcdDuration(u32 bcdDuration)
	{
		return TimeDuration::FromMilliseconds(s64(1000)
				* (SecondsPerHour * BcdValue((bcdDuration >> 16) & 0xff)
						+ SecondsPerMinute * BcdValue((bcdDuration >> 8) & 0xff)
						+ BcdValue(bcdDuration & 0xff)));
	}


	TimeZone::TimeZone(s16 minutes)
		: _minutesFromUtc(minutes)
	{ STINGRAYKIT_CHECK(minutes >= -12 * MinutesPerHour && minutes <= 14 * MinutesPerHour, ArgumentException("minutes", minutes)); }


	TimeZone TimeZone::Current()
	{ return TimeZone(TimeEngine::GetMinutesFromUtc()); }


	std::string TimeZone::ToString() const
	{
		const string_view sign = _minutesFromUtc > 0 ? "+" : _minutesFromUtc < 0 ? "-" : "";
		return StringBuilder() % sign % (Abs(_minutesFromUtc) / MinutesPerHour) % ":" % (Abs(_minutesFromUtc) % MinutesPerHour);
	}


	TimeZone TimeZone::FromString(string_view str)
	{
		STINGRAYKIT_CHECK(!str.empty(), FormatException(str));

		const optional<char> sign = str[0] == '+' || str[0] == '-' ? make_optional_value(str[0]) : null;
		const auto delimiterPos = str.find(':');

		int hours = 0;
		int minutes = 0;
		if (delimiterPos != string_view::npos)
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


	std::string Time::ToString(string_view format, TimeKind kind) const
	{ return BreakDown(kind).ToString(format); }


	Time Time::FromString(string_view str, TimeKind kind)
	{
		if (str == "now")
			return Time::Now();

		if (StartsWith(str, "now+"))
			return Time::Now() + TimeDuration::FromString(RemovePrefix(str, "now+"));

		s16 year = 0;
		s16 month = 0;
		s16 day = 0;
		s16 hour = 0;
		s16 minute = 0;
		TimeDuration seconds;
		char utcSign = 0;
		s16 utcHour = 0;
		s16 utcMinute = 0;

		bool haveDate = false;
		bool haveTime = false;
		bool haveSeconds = false;
		bool haveUtcSign = false;
		bool haveUtcHours = false;
		bool haveUtcMinutes = false;

		auto secondsProxy = MakeParseProxy(seconds, &ParseSeconds);

		if (StringParse(str, "%1%.%2%.%3% %4%:%5%:%6%", day, month, year, hour, minute, secondsProxy))
			haveDate = haveTime = haveSeconds = true;
		else if (StringParse(str, "%1%.%2%.%3% %4%:%5%", day, month, year, hour, minute))
			haveDate = haveTime = true;
		else if (StringParse(str, "%1%.%2%.%3%", day, month, year))
			haveDate = true;
		else if (StringParse(str, "%1%/%2%/%3% %4%:%5%:%6%", day, month, year, hour, minute, secondsProxy))
			haveDate = haveTime = haveSeconds = true;
		else if (StringParse(str, "%1%/%2%/%3% %4%:%5%", day, month, year, hour, minute))
			haveDate = haveTime = true;
		else if (StringParse(str, "%1%/%2%/%3%", day, month, year))
			haveDate = true;
		else if (StringParse(str, "%1%:%2%:%3%", hour, minute, secondsProxy))
			haveTime = haveSeconds = true;
		else if (StringParse(str, "%1%:%2%", hour, minute))
			haveTime = true;
		else if (StringParse(str, "%1%-%2%-%3%T%4%:%5%:%6%+%7%:%8%", year, month, day, hour, minute, secondsProxy, utcHour, utcMinute))
		{
			haveDate = haveTime = haveSeconds = haveUtcSign = haveUtcHours = haveUtcMinutes = true;
			utcSign = '+';
		}
		else if (StringParse(str, "%1%-%2%-%3%T%4%:%5%:%6%+%7%", year, month, day, hour, minute, secondsProxy, utcHour))
		{
			haveDate = haveTime = haveSeconds = haveUtcSign = haveUtcHours = true;
			utcSign = '+';
		}
		else if (StringParse(str, "%1%-%2%-%3%T%4%:%5%:%6%-%7%:%8%", year, month, day, hour, minute, secondsProxy, utcHour, utcMinute))
		{
			haveDate = haveTime = haveSeconds = haveUtcSign = haveUtcHours = haveUtcMinutes = true;
			utcSign = '-';
		}
		else if (StringParse(str, "%1%-%2%-%3%T%4%:%5%:%6%-%7%", year, month, day, hour, minute, secondsProxy, utcHour))
		{
			haveDate = haveTime = haveSeconds = haveUtcSign = haveUtcHours = true;
			utcSign = '-';
		}
		else if (StringParse(str, "%1%-%2%-%3%T%4%:%5%:%6%Z", year, month, day, hour, minute, secondsProxy))
		{
			haveDate = haveTime = haveSeconds = haveUtcSign = true;
			utcSign = 'Z';
		}
		else if (StringParse(str, "%1%-%2%-%3%T%4%:%5%:%6%", year, month, day, hour, minute, secondsProxy))
			haveDate = haveTime = haveSeconds = true;
		else if (StringParse(str, "%1%-%2%-%3%T%4%:%5%", year, month, day, hour, minute))
			haveDate = haveTime = true;
		else if (StringParse(str, "%1%-%2%-%3%", year, month, day))
			haveDate = true;
		else
			STINGRAYKIT_THROW(FormatException(str));

		STINGRAYKIT_CHECK(haveDate || haveTime, FormatException(str));
		STINGRAYKIT_CHECK(haveTime || !haveSeconds, FormatException(str));

		STINGRAYKIT_CHECK(
				!haveUtcSign
						|| ((utcSign == 'Z' && !haveUtcHours && !haveUtcMinutes)
								|| ((utcSign == '+' || utcSign == '-') && haveUtcHours)),
				FormatException(str));

		BrokenDownTime bdt;
		if (haveDate)
		{
			bdt.MonthDay = day;
			bdt.Month = month;
			bdt.Year = year > 100 ? year : year > 30 ? 1900 + year : 2000 + year;
		}
		else
		{
			bdt = Time::Now().BreakDown();
			bdt.Seconds = 0;
			bdt.Milliseconds = 0;
		}

		if (haveTime)
		{
			bdt.Hours = hour;
			bdt.Minutes = minute;
		}

		if (haveSeconds)
		{
			seconds = seconds.RoundToMilliseconds();

			bdt.Seconds = seconds.GetSeconds();
			bdt.Milliseconds = (seconds % TimeDuration::Second()).GetMilliseconds();
		}

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


	Time Time::MjdToEpoch(int mjd, u32 bcdDuration)
	{ return Time(s64(mjd - DaysSinceMjd) * SecondsPerDay * 1000) + TimeDuration::FromBcdDuration(bcdDuration); }


	int Time::GetMjd() const
	{ return DaysSinceMjd + _milliseconds / (1000 * SecondsPerDay); }


	u32 Time::GetBcdTime(TimeKind kind) const
	{
		const BrokenDownTime bdt(BreakDown(kind));
		return (BcdEncode(bdt.Hours) << 16) + (BcdEncode(bdt.Minutes) << 8) + BcdEncode(bdt.Seconds);
	}


	int Time::DaysTo(Time endTime) const
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
			TimeDuration		Seconds;
			TimeDuration		Offset;

			ParseResult()
				: Year(0), Month(0), Day(0), Hours(0), Minutes(0)
			{ }
		};

	public:
		Time operator () (string_view str) const
		{
			const std::string uppercase = ToUpper(str); // Rfc3339 5.6 NOTE

			optional<ParseResult> result = TryFromDateTime(uppercase);

			if (!result)
				result = TryFromDateTimeWithOffset(uppercase);

			if (!result)
				result = TryFromDate(uppercase);

			STINGRAYKIT_CHECK(result, FormatException(str));

			const TimeDuration seconds = result->Seconds.RoundToMilliseconds();
			const BrokenDownTime brokenDown((seconds % TimeDuration::Second()).GetMilliseconds(), seconds.GetSeconds(), result->Minutes, result->Hours, 0, result->Day, result->Month, 0, result->Year);
			const Time local = Time::FromBrokenDownTime(brokenDown, TimeKind::Utc);

			return local - result->Offset;
		}

	private:
		static optional<ParseResult> TryFromDateTime(string_view str)
		{
			ParseResult result;
			auto secondsProxy = MakeParseProxy(result.Seconds, &ParseSeconds);

			if (!StringParse(str, "%1%-%2%-%3%T%4%:%5%:%6%Z", result.Year, result.Month, result.Day, result.Hours, result.Minutes, secondsProxy)
					&& !StringParse(str, "%1%-%2%-%3%T%4%:%5%:%6%", result.Year, result.Month, result.Day, result.Hours, result.Minutes, secondsProxy))
				return null;

			return result;
		}

		static optional<ParseResult> TryFromDateTimeWithOffset(string_view str)
		{
			ParseResult result;

			const size_t SignPosFromEnd = 5;
			if (str.size() <= SignPosFromEnd)
				return null;

			const char sign = str[str.size() - 1 - SignPosFromEnd];
			const s8 multiplier = sign == '+' ? 1 : sign == '-' ? -1 : 0;
			if (multiplier == 0)
				return null;

			char format[] = "%1%-%2%-%3%T%4%:%5%:%6% %7%:%8%";
			format[StrLen(format) - 8] = sign;

			s16 offsetHours = 0;
			s16 offsetMinutes = 0;
			const bool success = StringParse(
					str,
					format,
					result.Year,
					result.Month,
					result.Day,
					result.Hours,
					result.Minutes,
					*MakeParseProxy(result.Seconds, &ParseSeconds),
					offsetHours,
					offsetMinutes);

			if (!success)
				return null;

			result.Offset = TimeDuration::FromHours(offsetHours) + TimeDuration::FromMinutes(offsetMinutes);
			result.Offset *= multiplier;

			return result;
		}

		static optional<ParseResult> TryFromDate(string_view str)
		{
			ParseResult result;
			if (!StringParse(str, "%1%-%2%-%3%", result.Year, result.Month, result.Day))
				return null;

			return result;
		}
	};


	int TimeCmp::operator () (Time lhs, Time rhs) const
	{
		const TimeDuration distance = lhs > rhs ? lhs - rhs : rhs - lhs;
		if (distance <= _allowedJitter)
			return 0;

		return comparers::Cmp()(lhs, rhs);
	}


	std::string TimeUtility::ToIso8601(Time time)
	{ return time.BreakDown(TimeKind::Utc).ToString("YYYY-MM-ddThh:mm:ss.lllZ"); }


	Time TimeUtility::FromIso8601(string_view str)
	{ return FromIso8601Impl()(str); }


	class TimeDurationUtility::FromIso8601Impl
	{
		static const std::regex FormatRegex;
		static const std::regex DateTimeRegex;

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
		TimeDuration operator () (string_view str, Time base) const
		{
			const std::string uppercase = ToUpper(str);
			const string_view uppercaseView = uppercase;

			ParseResult result;

			svmatch match;
			STINGRAYKIT_CHECK(std::regex_match(uppercaseView.begin(), uppercaseView.end(), match, FormatRegex), FormatException(str));
			STINGRAYKIT_CHECK(TryFromDateTime(svmatch_str(match, 1), result) || TryFromWeek(svmatch_str(match, 1), result), FormatException(str));

			return result.ToTimeDuration(base, TimeKind::Utc);
		}

	private:
		static bool TryFromDateTime(string_view str, ParseResult& result)
		{
			svmatch match;
			if (std::regex_match(str.begin(), str.end(), match, DateTimeRegex))
				return match[2].first != match[2].second
						&& TryFromTime(svmatch_str(match, 2), result)
						&& (match[1].first == match[1].second || TryFromDate(svmatch_str(match, 1), result));
			else
				return TryFromDate(str, result);
		}

		static bool TryFromWeek(string_view str, ParseResult& result)
		{ return StringParse(str, "%1%W", result.Weeks); }

		static bool TryFromTime(string_view str, ParseResult& result)
		{
			string_view toParse = str;
			string_view remaining;

			if (Contains(toParse, "H"))
			{
				if (EndsWith(toParse, "H"))
					return StringParse(toParse, "%1%H", result.Hours);

				if (!StringParse(toParse, "%1%H%2%", result.Hours, remaining))
					return false;

				toParse = remaining;
				remaining = string_view();
			}

			if (Contains(toParse, "M"))
			{
				if (EndsWith(toParse, "M"))
					return StringParse(toParse, "%1%M", result.Minutes);

				if (!StringParse(toParse, "%1%M%2%", result.Minutes, remaining))
					return false;

				toParse = remaining;
				remaining = string_view();
			}

			return StringParse(toParse, "%1%S", result.Seconds);
		}

		static bool TryFromDate(string_view str, ParseResult& result)
		{
			string_view toParse = str;
			string_view remaining;

			if (Contains(toParse, "Y"))
			{
				if (EndsWith(toParse, "Y"))
					return StringParse(toParse, "%1%Y", result.Years);

				if (!StringParse(toParse, "%1%Y%2%", result.Years, remaining))
					return false;

				toParse = remaining;
				remaining = string_view();
			}

			if (Contains(toParse, "M"))
			{
				if (EndsWith(toParse, "M"))
					return StringParse(toParse, "%1%M", result.Months);

				if (!StringParse(toParse, "%1%M%2%", result.Months, remaining))
					return false;

				toParse = remaining;
				remaining = string_view();
			}

			return StringParse(toParse, "%1%D", result.Days);
		}
	};

	const std::regex TimeDurationUtility::FromIso8601Impl::FormatRegex(R"(P([WYMDTHS\d]{2,}))", std::regex::optimize);
	const std::regex TimeDurationUtility::FromIso8601Impl::DateTimeRegex(R"((.*)T(.*))", std::regex::optimize);


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


	TimeDuration TimeDurationUtility::FromIso8601(string_view str, Time base)
	{ return FromIso8601Impl()(str, base); };

}
