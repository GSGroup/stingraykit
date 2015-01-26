#ifndef STINGRAYKIT_TIME_TIME_H
#define STINGRAYKIT_TIME_TIME_H


#include <stingraykit/serialization/ISerializable.h>
#include <stingraykit/time/BrokenDownTime.h>
#include <stingraykit/toolkit.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_time
	 * @{
	 */

	class TimeDuration
	{
	private:
		s64		_microseconds;

	private:
		TimeDuration(s64 microseconds, Dummy dummy) : _microseconds(microseconds)
		{ }

	public:
		explicit TimeDuration(s64 milliseconds = 0) : _microseconds(1000 * milliseconds)
		{ }

		s64 GetMilliseconds() const							{ return _microseconds / 1000; }
		s64 GetMicroseconds() const							{ return _microseconds; }

		TimeDuration Absolute() const						{ return TimeDuration(_microseconds < 0 ? -_microseconds : _microseconds, Dummy()); }

		void Serialize(ObjectOStream & ar) const;
		void Deserialize(ObjectIStream & ar);

		TimeDuration operator+=(TimeDuration other)			{ _microseconds += other._microseconds; return *this; }
		TimeDuration operator+(TimeDuration other) const 	{ TimeDuration result(*this); return result += other; }

		TimeDuration operator-=(TimeDuration other)			{ _microseconds -= other._microseconds; return *this; }
		TimeDuration operator-(TimeDuration other) const 	{ TimeDuration result(*this); return result -= other; }
		TimeDuration operator-() const 						{ return TimeDuration(-GetMilliseconds()); }

		TimeDuration operator*=(int multiplier)				{ _microseconds *= multiplier; return *this; }
		TimeDuration operator*(int multiplier) const		{ TimeDuration result(*this); return result *= multiplier; }

		TimeDuration operator/=(int multiplier)				{ _microseconds /= multiplier; return *this; }
		TimeDuration operator/(int multiplier) const		{ TimeDuration result(*this); return result /= multiplier; }


		bool operator < (TimeDuration other) const			{ return _microseconds < other._microseconds; }
		STINGRAYKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(TimeDuration);

		std::string ToString(const std::string& format = "") const;
		static TimeDuration FromString(const std::string& s);

		static TimeDuration FromMicroseconds(s64 microseconds)	{ return TimeDuration(microseconds, Dummy()); }
		static TimeDuration FromMilliseconds(s64 milliseconds)	{ return TimeDuration(milliseconds); }

		static TimeDuration Second()	{ return TimeDuration(1000); }
		static TimeDuration Minute()	{ return Second() * 60; }
		static TimeDuration Hour()		{ return Minute() * 60; }
		static TimeDuration Day()		{ return Hour() * 24; }

		static TimeDuration FromSeconds(int seconds)	{ return Second() * seconds; }
		static TimeDuration FromMinutes(int minutes)	{ return Minute() * minutes; }
		static TimeDuration FromHours(int hours)		{ return Hour() * hours; }
		static TimeDuration FromDays(int days)			{ return Day() * days; }
	};


	class TimeZone
	{
	private:
		s16		_minutesFromUtc;

	public:
		TimeZone();
		explicit TimeZone(s16 minutes);

		s16 GetMinutesFromUtc() const { return _minutesFromUtc; }
		void SetMinutesFromUtc(s16 minutes) { _minutesFromUtc = minutes; }

		static TimeZone Current();

		bool operator < (const TimeZone& other) const { return _minutesFromUtc < other._minutesFromUtc; }
		STINGRAYKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(TimeZone);

		std::string ToString() const;
		static TimeZone FromString(const std::string& str);

		void Serialize(ObjectOStream & ar) const;
		void Deserialize(ObjectIStream & ar);
	};


	class TimeKind
	{
		STINGRAYKIT_ENUM_VALUES(Utc, Local);
		STINGRAYKIT_DECLARE_ENUM_CLASS(TimeKind);
	};


	class Time
	{
	private:
		s64		_milliseconds;

	public:
		Time();
		explicit Time(s64 milliseconds);

		static Time FromTimeT(time_t t)
		{ return Time((s64)t * 1000); }

		u32 ToTime_t() const
		{ return _milliseconds / 1000; }

		TimeDuration operator - (const Time& other) const
		{ return TimeDuration(_milliseconds - other._milliseconds); }

		Time& operator -= (TimeDuration duration)
		{
			_milliseconds -= duration.GetMilliseconds();
			return *this;
		}

		Time& operator += (TimeDuration duration)
		{
			_milliseconds += duration.GetMilliseconds();
			return *this;
		}

		Time operator - (TimeDuration duration) const
		{
			Time result(*this);
			return result -= duration;
		}


		Time operator + (TimeDuration duration) const
		{
			Time result(*this);
			return result += duration;
		}


		bool operator < (const Time& other) const
		{ return _milliseconds < other._milliseconds; }

		STINGRAYKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(Time);


		static Time Now();

		s64 GetMilliseconds() const { return _milliseconds; }

		BrokenDownTime BreakDown(TimeKind kind = TimeKind::Local) const;
		static Time FromBrokenDownTime(const BrokenDownTime& bdt, TimeKind kind = TimeKind::Local);

		std::string ToString(const std::string& format = "", TimeKind kind = TimeKind::Local) const;
		static Time FromString(const std::string& s, TimeKind kind = TimeKind::Local);

		static Time MJDtoEpoch(int mjd, u32 bcdTime = 0);
		static TimeDuration BCDDurationToTimeDuration(u32 bcdTime);

		int GetMJD() const;
		u32 GetBCDTime(TimeKind kind = TimeKind::Local) const;

		void Serialize(ObjectOStream & ar) const;
		void Deserialize(ObjectIStream & ar);

		int DaysTo(const Time& endTime);
	};

	/** @} */

}


#endif
