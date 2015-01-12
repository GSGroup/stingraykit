#include <stingray/toolkit/time/Time.h>

#include <stdio.h>

#include <stingray/toolkit/serialization/Serialization.h>
#include <stingray/toolkit/time/TimeEngine.h>
#include <stingray/toolkit/string/StringFormat.h>
#include <stingray/toolkit/exception.h>

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

		BrokenDownTime bdt = Time::Now().BreakDown();
		int a, b, c, d, e, f;
		int components = 0;
		int otherComponents = 0;

		components = sscanf(s.c_str(), "%d.%d.%d %d:%d:%d", &a, &b, &c, &d, &e, &f);
		otherComponents = sscanf(s.c_str(), "%d/%d/%d", &a, &b, &c);

		if (otherComponents > components)
		{
			if (otherComponents == 3)
			{
				bdt.MonthDay		= c;
				bdt.Month			= b;
				bdt.Year			= (a > 100 ? a : (a > 30 ? 1900 + a : 2000 + a));

				bdt.Hours			= 0;
				bdt.Minutes			= 0;
				bdt.Seconds			= 0;
				bdt.Milliseconds	= 0;
			}
			else
				STINGRAYKIT_THROW("Could not parse Time!");
		}
		else
		{
			if (components >= 3)
			{
				bdt.MonthDay		= a;
				bdt.Month			= b;
				bdt.Year			= (c > 100 ? c : (c > 30 ? 1900 + c : 2000 + c));
				components -= 3;
			}
			else
				components = sscanf(s.c_str(), "%d:%d:%d", &d, &e, &f);

			if (components >= 2)
			{
				bdt.Hours			= d;
				bdt.Minutes			= e;
				bdt.Seconds			= (components == 2) ? 0 : f;
				bdt.Milliseconds	= 0;
			}
			else
				STINGRAYKIT_THROW("Could not parse Time!");
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
	{ STINGRAYKIT_CHECK(_minutesFromUtc >= -12 * MinutesPerHour && _minutesFromUtc <= 14 * MinutesPerHour, IndexOutOfRangeException()); }

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

}
