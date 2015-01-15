#include <stingraykit/time/ElapsedTime.h>

#include <stingraykit/time/TimeEngine.h>


namespace stingray
{

	ElapsedTime::ElapsedTime()
		: _timer(TimeEngine::CreateMonotonicTimer())
	{
		_start = _timer.GetMicroseconds();
	}

	TimeDuration ElapsedTime::Restart()
	{
		s64 newStart = _timer.GetMicroseconds();
		TimeDuration result(TimeDuration::FromMicroseconds(newStart - _start));
		_start = newStart;
		return result;
	}

	s64 ElapsedTime::ElapsedMilliseconds() const
	{
		return ElapsedMicroseconds() / 1000;
	}

	s64 ElapsedTime::ElapsedMicroseconds() const
	{
		return (s64)_timer.GetMicroseconds() - _start;
	}

	TimeDuration ElapsedTime::Elapsed() const
	{ return TimeDuration(ElapsedMilliseconds()); }

}
