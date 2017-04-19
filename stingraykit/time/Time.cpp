// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/time/Time.h>

#include <stdio.h>

#include <stingraykit/exception.h>
#include <stingraykit/serialization/Serialization.h>
#include <stingraykit/string/StringFormat.h>
#include <stingraykit/string/StringParse.h>
#include <stingraykit/string/StringUtils.h>
#include <stingraykit/time/TimeEngine.h>
#include <stingraykit/log/Logger.h>

namespace stingray
{

	static const int SecondsPerMinute		= 60;
	static const int MillisecondsPerMinute	= SecondsPerMinute * 1000;
	static const int MinutesPerHour			= 60;
	static const int SecondsPerHour			= MinutesPerHour * SecondsPerMinute;
	static const int SecondsPerDay			= 24 * SecondsPerHour;
	static const int DaysSinceMjd			= 40587;


	void TimeDuration::Serialize(ObjectOStream & ar) const
	{ ar.Serialize("us", _microseconds); }


	void TimeDuration::Deserialize(ObjectIStream & ar)
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
		bool has_hours = std::search(format.begin(), format.end(), hours_placeholder.begin(), hours_placeholder.end()) != format.end();

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


	Time::Time()
		: _milliseconds(0)
	{ }

	Time::Time(s64 milliseconds)
		: _milliseconds(milliseconds)
	{ }

	Time Time::Now() { return Time(TimeEngine::GetMillisecondsSinceEpoch()); }

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

	void Time::Serialize(ObjectOStream & ar) const	{ ar.Serialize("ms", _milliseconds); }
	void Time::Deserialize(ObjectIStream & ar)		{ ar.Deserialize("ms", _milliseconds); }

	TimeZone::TimeZone()
		: _minutesFromUtc()
	{ }

	TimeZone::TimeZone(s16 minutes)
		: _minutesFromUtc(minutes)
	{ STINGRAYKIT_CHECK(minutes >= -12 * MinutesPerHour && minutes <= 14 * MinutesPerHour, ArgumentException("minutes", minutes)); }

	TimeZone TimeZone::Current()
	{ return TimeZone(TimeEngine::GetMinutesFromUtc()); }

	std::string TimeZone::ToString() const
	{
		const std::string sign = _minutesFromUtc > 0? "+" : _minutesFromUtc < 0? "-" : "";
		return StringBuilder() % sign % (_minutesFromUtc / MinutesPerHour) % ":" % (_minutesFromUtc % MinutesPerHour);
	}

	TimeZone TimeZone::FromString(const std::string& str)
	{
		char sign;
		int hours, minutes;

		STINGRAYKIT_CHECK(sscanf(str.c_str(), "%c%d:%d", &sign, &hours, &minutes) == 3, FormatException());

		const int value = hours * MinutesPerHour + minutes;
		return TimeZone(sign == '+'? value : -value);
	}


	void TimeZone::Serialize(ObjectOStream & ar) const	{ ar.Serialize("offset", _minutesFromUtc); }
	void TimeZone::Deserialize(ObjectIStream & ar)		{ ar.Deserialize("offset", _minutesFromUtc); }


	const s64 SecondsBetweenNtpAndUnixEpochs = 2208988800ll;

	u64 Time::ToNtpTimestamp() const
	{
		return GetMilliseconds() / 1000 + SecondsBetweenNtpAndUnixEpochs;
	}

	Time Time::FromNtpTimestamp(u64 timestamp)
	{
		return Time((timestamp - SecondsBetweenNtpAndUnixEpochs) * 1000);
	}

	static inline u8 bcdValue(u8 byte)
	{ return ((byte >> 4) & 0x0f) * 10 + (byte & 0x0f); }

	static inline u8 bcdEncode(u8 value)
	{ return ((value / 10) << 4) + value % 10; }

	Time Time::MJDtoEpoch(int mjd, u32 bcdTime)
	{ return Time(s64(mjd - DaysSinceMjd) * SecondsPerDay * 1000) + BCDDurationToTimeDuration(bcdTime); }


	TimeDuration Time::BCDDurationToTimeDuration(u32 bcdTime)
	{
		return TimeDuration(s64(1000) * (SecondsPerHour * bcdValue((bcdTime >> 16) & 0xff) +
				SecondsPerMinute * bcdValue((bcdTime >> 8) & 0xff) +
				bcdValue(bcdTime & 0xff)));
	}

	int Time::GetMJD() const
	{
		return DaysSinceMjd + _milliseconds / (1000 * SecondsPerDay);
	}

	u32 Time::GetBCDTime(TimeKind kind) const
	{
		BrokenDownTime bdt(this->BreakDown(kind));
		return (bcdEncode(bdt.Hours) << 16) + (bcdEncode(bdt.Minutes) << 8) + bcdEncode(bdt.Seconds);
	}

	int Time::DaysTo(const Time& endTime)
	{
		return (Time::FromBrokenDownTime((*this).BreakDown().GetDayStart()).GetMilliseconds() - Time::FromBrokenDownTime(endTime.BreakDown().GetDayStart()).GetMilliseconds()) / 86400000;	// 1000 * 60 * 60 * 24 = 86400000
	}


	namespace TimeUtility
	{

		std::string ToIso8601(const Time& time)
		{ return time.BreakDown(TimeKind::Utc).ToString("YYYY-MM-ddThh:mm:ss.lllZ"); }


		Time FromIso8601(const std::string& format)
		{
			const std::string uppercase = ToUpper(format); // Rfc3339 5.6 NOTE

			s16 year;
			s16 month;
			s16 day;
			s16 hours;
			s16 minutes;
			double seconds;
			TimeDuration offset;

			if (EndsWith(uppercase, "Z"))
				STINGRAYKIT_CHECK(StringParse(uppercase, "%1%-%2%-%3%T%4%:%5%:%6%Z",
					year,
					month,
					day,
					hours,
					minutes,
					seconds),
				FormatException(uppercase));
			else
			{
				const size_t SignPosFromEnd = 5;
				STINGRAYKIT_CHECK(uppercase.size() > SignPosFromEnd, FormatException(uppercase));
				const char sign = uppercase[uppercase.size() - 1 - SignPosFromEnd];

				const s8 multiplier = sign == '+' ? 1 : (sign == '-' ? -1 : 0);
				STINGRAYKIT_CHECK(multiplier, FormatException(uppercase));

				s16 offsetHours;
				s16 offsetMinutes;
				STINGRAYKIT_CHECK(StringParse(uppercase, StringBuilder() % "%1%-%2%-%3%T%4%:%5%:%6%" % sign % "%7%:%8%",
					year,
					month,
					day,
					hours,
					minutes,
					seconds,
					offsetHours,
					offsetMinutes),
				FormatException(uppercase));

				offset = TimeDuration::FromHours(offsetHours) + TimeDuration::FromMinutes(offsetMinutes);
				offset *= multiplier;
			}

			const s16 milliseconds = (seconds - double(s16(seconds))) * 1000.;
			const BrokenDownTime brokenDown(milliseconds, (s16)seconds, minutes, hours, 0, day, month, 0, year);
			const Time local = Time::FromBrokenDownTime(brokenDown, TimeKind::Utc);
			return local - offset;
		}

	}

}
