// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
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
#include <stingraykit/log/Logger.h>
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

		u8 BcdValue(u8 byte)
		{ return ((byte >> 4) & 0x0f) * 10 + (byte & 0x0f); }

		u8 BcdEncode(u8 value)
		{ return ((value / 10) << 4) + value % 10; }

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

		std::string hours_placeholder = "hh";
		bool has_hours = std::search(result.begin(), result.end(), hours_placeholder.begin(), hours_placeholder.end()) != result.end();

		s64 abs_ms = Absolute().GetMilliseconds();
		s64 hours = has_hours ? abs_ms / Hour().GetMilliseconds() : 0;
		s64 minutes = (abs_ms - hours * Hour().GetMilliseconds()) / Minute().GetMilliseconds();
		s64 seconds = abs_ms % Minute().GetMilliseconds() / Second().GetMilliseconds();
		s64 milliseconds = abs_ms % Second().GetMilliseconds();

		if (has_hours)
			ReplaceAll(result, "hh", StringFormat("%1$2%", hours));
		else
			ReplaceAll(result, "hh", "");

		ReplaceAll(result, "mm", StringFormat("%1$2%", minutes));
		ReplaceAll(result, "ss", StringFormat("%1$2%", seconds));
		ReplaceAll(result, "lll", StringFormat("%1$3%", milliseconds));
		return result;
	}


	TimeDuration TimeDuration::FromString(const std::string& s)
	{
		int n;
		char c = 0;
		int components = sscanf(s.c_str(), "%d%c", &n, &c);
		if (components < 1)
			STINGRAYKIT_THROW("Invalid time duration format");
		switch (c)
		{
		case 'H':
		case 'h':
			return FromHours(n);
		case 'M':
		case 'm':
			return FromMinutes(n);
		case 'S':
		case 's':
			return FromSeconds(n);
		case 0:
			return TimeDuration(n);
		};
		STINGRAYKIT_THROW("Could not parse TimeDuration!");
	}


	TimeZone::TimeZone(s16 minutes)
		: _minutesFromUtc(minutes)
	{ STINGRAYKIT_CHECK(minutes >= -12 * MinutesPerHour && minutes <= 14 * MinutesPerHour, ArgumentException("minutes", minutes)); }


	TimeZone TimeZone::Current()
	{ return TimeZone(TimeEngine::GetMinutesFromUtc()); }


	std::string TimeZone::ToString() const
	{
		const std::string sign = _minutesFromUtc > 0 ? "+" : (_minutesFromUtc < 0 ? "-" : "");
		return StringBuilder() % sign % (Abs(_minutesFromUtc) / MinutesPerHour) % ":" % (Abs(_minutesFromUtc) % MinutesPerHour);
	}


	TimeZone TimeZone::FromString(const std::string& str)
	{
		STINGRAYKIT_CHECK(!str.empty(), FormatException(str));

		const optional<char> sign = str[0] == '+' || str[0] == '-' ? str[0] : optional<char>();

		int hours, minutes;
		STINGRAYKIT_CHECK(sscanf(str.c_str() + (sign ? 1 : 0), "%d:%d", &hours, &minutes) == 2, FormatException(str));

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
		const s64 offset = kind == TimeKind::Utc? 0 : MillisecondsPerMinute * TimeEngine::GetMinutesFromUtc();
		return TimeEngine::BrokenDownFromMilliseconds(_milliseconds + offset);
	}


	Time Time::FromBrokenDownTime(const BrokenDownTime& bdt, TimeKind kind)
	{
		const s64 offset = kind == TimeKind::Utc? 0 : MillisecondsPerMinute * TimeEngine::GetMinutesFromUtc();
		return Time(TimeEngine::MillisecondsFromBrokenDown(bdt) - offset);
	}


	std::string Time::ToString(const std::string& format, TimeKind kind) const
	{ return BreakDown(kind).ToString(format); }


	Time Time::FromString(const std::string& s, TimeKind kind)
	{
		if (s == "now")
			return Time::Now();
		if (s.size() > 4 && s.substr(0, 4) == "now+")
			return Time::Now() + TimeDuration::FromString(s.substr(4));

		s16 year, month, day;
		s16 hour, minute, second;
		char utcSign;
		s16 utcHour, utcMinute;
		bool haveDate = false;
		bool haveTime = false;
		bool haveSeconds = false;
		bool haveUtcSign = false;
		bool haveUtcHours = false;
		bool haveUtcMinutes = false;

		int components = sscanf(s.c_str(), "%hd.%hd.%hd %hd:%hd:%hd", &day, &month, &year, &hour, &minute, &second);
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
			components = sscanf(s.c_str(), "%hd/%hd/%hd %hd:%hd:%hd", &day, &month, &year, &hour, &minute, &second);
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
				components = sscanf(s.c_str(), "%hd:%hd:%hd", &hour, &minute, &second);
				if (components >= 2)
				{
					haveTime = true;
					if (components >= 3)
						haveSeconds = true;
				}
				else
				{
					components = sscanf(s.c_str(), "%hd-%hd-%hdT%hd:%hd:%hd%c%hd:%hd", &year, &month, &day, &hour, &minute, &second, &utcSign, &utcHour, &utcMinute);
					if (components >= 3)
					{
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
					else
						STINGRAYKIT_THROW("Unknown time format!");
				}
			}
		}
		STINGRAYKIT_CHECK((haveDate || haveTime), "Could not parse Time!");
		STINGRAYKIT_CHECK(!(!haveTime && haveSeconds), "Have seconds without hours and minutes!");
		STINGRAYKIT_CHECK(!haveUtcSign || ((utcSign == 'Z' && !haveUtcHours && !haveUtcMinutes) || ((utcSign == '+' || utcSign == '-') && haveUtcHours && !(!haveUtcHours && haveUtcMinutes))), "Malformed UTC suffix");

		if (haveUtcSign)
			Logger::Debug() << "Time::FromString: time kind parameter will be ignored because time string have UTC sign";

		BrokenDownTime bdt;
		if (haveDate)
		{
			bdt.MonthDay		= day;
			bdt.Month			= month;
			bdt.Year			= (year > 100 ? year : (year > 30 ? 1900 + year : 2000 + year));
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
			kind = TimeKind::Utc;
			if ((utcSign == '+') || (utcSign == '-'))
			{
				s16 minutesFromUtc = (haveUtcHours ? (utcHour * MinutesPerHour) : 0) + (haveUtcMinutes ? utcMinute : 0);
				if (utcSign == '-')
					bdt.Minutes += minutesFromUtc;
				else
					bdt.Minutes -= minutesFromUtc;
			}
			else if (utcSign != 'Z')
				STINGRAYKIT_THROW("Unknown UTC sign!");
		}

		return FromBrokenDownTime(bdt, kind);
	}


	u64 Time::ToNtpTimestamp() const
	{ return GetMilliseconds() / 1000 + SecondsBetweenNtpAndUnixEpochs; }


	Time Time::FromNtpTimestamp(u64 timestamp)
	{ return Time((timestamp - SecondsBetweenNtpAndUnixEpochs) * 1000); }


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
	{ return (Time::FromBrokenDownTime((*this).BreakDown().GetDayStart()).GetMilliseconds() - Time::FromBrokenDownTime(endTime.BreakDown().GetDayStart()).GetMilliseconds()) / (24 * 60 * 60 * 1000); }


	void Time::Serialize(ObjectOStream& ar) const
	{ ar.Serialize("ms", _milliseconds); }


	void Time::Deserialize(ObjectIStream& ar)
	{ ar.Deserialize("ms", _milliseconds); }


	namespace TimeUtility
	{

		struct FromIso8601Impl
		{
			struct ParseResult
			{
				s16 Year;
				s16 Month;
				s16 Day;
				s16 Hours;
				s16 Minutes;
				double Seconds;
				TimeDuration Offset;

				ParseResult()
					:	Year(),
						Month(),
						Day(),
						Hours(),
						Minutes(),
						Seconds(),
						Offset()
				{ }
			};

			Time operator()(const std::string& format) const
			{
				const std::string uppercase = ToUpper(format); // Rfc3339 5.6 NOTE

				optional<ParseResult> result;
				if (!(result = TryFromDateTime(uppercase)))
					if (!(result = TryFromDateTimeWithOffset(uppercase)))
						if (!(result = TryFromDate(uppercase)))
							STINGRAYKIT_THROW(FormatException(uppercase));

				const s16 milliseconds = (result->Seconds - double(s16(result->Seconds))) * 1000.;
				const BrokenDownTime brokenDown(milliseconds, (s16)result->Seconds, result->Minutes, result->Hours, 0, result->Day, result->Month, 0, result->Year);
				const Time local = Time::FromBrokenDownTime(brokenDown, TimeKind::Utc);

				return local - result->Offset;
			}

		private:
			optional<ParseResult> TryFromDateTime(const std::string& format) const
			{
				ParseResult result;
				if (!StringParse(format, "%1%-%2%-%3%T%4%:%5%:%6%Z", result.Year, result.Month, result.Day, result.Hours, result.Minutes, result.Seconds))
					if (!StringParse(format, "%1%-%2%-%3%T%4%:%5%:%6%", result.Year, result.Month, result.Day, result.Hours, result.Minutes, result.Seconds))
						return null;

				return result;
			}

			optional<ParseResult> TryFromDateTimeWithOffset(const std::string& format) const
			{
				ParseResult result;

				const size_t SignPosFromEnd = 5;
				if (format.size() <= SignPosFromEnd)
					return null;

				const char sign = format[format.size() - 1 - SignPosFromEnd];
				const s8 multiplier = sign == '+' ? 1 : (sign == '-' ? -1 : 0);
				if (multiplier == 0)
					return null;

				s16 offsetHours;
				s16 offsetMinutes;
				const bool success = StringParse(format, StringBuilder() % "%1%-%2%-%3%T%4%:%5%:%6%" % sign % "%7%:%8%",
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

			optional<ParseResult> TryFromDate(const std::string& format) const
			{
				ParseResult result;
				if (!StringParse(format, "%1%-%2%-%3%", result.Year, result.Month, result.Day))
					return null;

				return result;
			}
		};


		std::string ToIso8601(const Time& time)
		{ return time.BreakDown(TimeKind::Utc).ToString("YYYY-MM-ddThh:mm:ss.lllZ"); }


		Time FromIso8601(const std::string& format)
		{ return FromIso8601Impl()(format); }

	}


	namespace TimeDurationUtility
	{

		struct FromIso8601Impl
		{
			struct ParseResult
			{
				s16 Years;
				s16 Months;
				s16 Weeks;
				s16 Days;
				s16 Hours;
				s16 Minutes;
				s16 Seconds;

				ParseResult()
					: Years(0), Months(0), Weeks(0), Days(0), Hours(0), Minutes(0), Seconds(0)
				{ }

				TimeDuration ToTimeDuration(Time base, TimeKind kind) const
				{
					if (Years == 0 && Months == 0)
						return TimeDuration::FromDays(Weeks * DaysPerWeek + Days) + TimeDuration::FromHours(Hours) + TimeDuration::FromMinutes(Minutes) + TimeDuration::FromSeconds(Seconds);

					const BrokenDownTime bdt = base.BreakDown(kind);
					return Time::FromBrokenDownTime(BrokenDownTime(
							bdt.Milliseconds,
							bdt.Seconds + Seconds,
							bdt.Minutes + Minutes,
							bdt.Hours + Hours,
							bdt.WeekDay,
							bdt.MonthDay + Weeks * DaysPerWeek + Days,
							bdt.Month + Months,
							bdt.YearDay,
							bdt.Year + Years), kind) - base;
				}
			};

		private:
			ParseResult _result;

		public:
			TimeDuration operator()(const std::string& format, Time base)
			{
				const std::string uppercase = ToUpper(format);

				smatch m;
				if (!regex_search(uppercase, m, regex("^P([0-9WYMDTHS]{2,})$")))
					STINGRAYKIT_THROW(FormatException(format));

				if (!TryFromDateTime(m[1]) && !TryFromWeek(m[1]))
					STINGRAYKIT_THROW(FormatException(format));

				return _result.ToTimeDuration(base, TimeKind::Utc);
			}

		private:
			bool TryFromDateTime(const std::string& format)
			{
				smatch m;
				if (regex_search(format, m, regex("^(.*)T(.*)$")))
					return !m[2].empty() && TryFromTime(m[2]) && (m[1].empty() || TryFromDate(m[1]));
				else
					return TryFromDate(format);
			}

			bool TryFromWeek(const std::string& format)
			{ return StringParse(format, "%1%W", _result.Weeks); }

			bool TryFromTime(std::string format)
			{
				if (Contains(format, "H"))
				{
					if (EndsWith(format, "H"))
						return StringParse(format, "%1%H", _result.Hours);

					if (!StringParse(format, "%1%H%2%", _result.Hours, format))
						return false;
				}

				if (Contains(format, "M"))
				{
					if (EndsWith(format, "M"))
						return StringParse(format, "%1%M", _result.Minutes);

					if (!StringParse(format, "%1%M%2%", _result.Minutes, format))
						return false;
				}

				return StringParse(format, "%1%S", _result.Seconds);
			}

			bool TryFromDate(std::string format)
			{
				if (Contains(format, "Y"))
				{
					if (EndsWith(format, "Y"))
						return StringParse(format, "%1%Y", _result.Years);

					if (!StringParse(format, "%1%Y%2%", _result.Years, format))
						return false;
				}

				if (Contains(format, "M"))
				{
					if (EndsWith(format, "M"))
						return StringParse(format, "%1%M", _result.Months);

					if (!StringParse(format, "%1%M%2%", _result.Months, format))
						return false;
				}

				return StringParse(format, "%1%D", _result.Days);
			}
		};


		int GetMaxDaysInYear(int year)
		{ return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0) ? 366 : 365; }


		std::string ToIso8601(TimeDuration timeDuration, const optional<Time>& base)
		{
			STINGRAYKIT_CHECK(timeDuration.GetMicroseconds() >= 0, ArgumentException("timeDuration", timeDuration));

			s16 Years, Months, Days, Hours, Minutes, Seconds;

			if (!base)
			{
				Years	= 0;
				Months	= 0;
				Days	= 0;
				Hours	= timeDuration.GetHours();
				Minutes	= timeDuration.GetMinutes() % MinutesPerHour;
				Seconds	= timeDuration.GetSeconds() % SecondsPerMinute;
			}
			else
			{
				Years	= 0;
				Months	= 0;
				Days	= timeDuration.GetDays();
				Hours	= timeDuration.GetHours() % HoursPerDay;
				Minutes	= timeDuration.GetMinutes() % MinutesPerHour;
				Seconds	= timeDuration.GetSeconds() % SecondsPerMinute;

				BrokenDownTime bdt = base->BreakDown(TimeKind::Utc);
				int size;

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


		TimeDuration FromIso8601(const std::string& format, Time base)
		{ return FromIso8601Impl()(format, base); };

	}

}
