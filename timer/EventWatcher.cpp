#include <stingray/toolkit/timer/EventWatcher.h>


namespace stingray
{

	EventWatcher::EventWatcher(Timer& timer, size_t timeout, size_t frequency)
		: _deferrer(new ExecutionDeferrer(timer, timeout)), _frequency(frequency), _count(0)
	{
		STINGRAYKIT_CHECK(frequency, ArgumentException("frequency"));
		_deferrer->Defer(bind(&EventWatcher::Timeout, this));
	}

	EventWatcher::~EventWatcher()
	{ _deferrer.reset(); }


	void EventWatcher::Touch()
	{
		_deferrer->Defer(bind(&EventWatcher::Timeout, this));

		signal_locker l1(OnOccured), l2(OnTimeout);
		if (++_count == _frequency)
		{
			_count = 0;
			OnOccured();
		}
	}


	void EventWatcher::Timeout()
	{
		signal_locker l1(OnOccured), l2(OnTimeout);
		_count = _frequency - 1;
		OnTimeout();
	}


	EventWatcherWithTimer::EventWatcherWithTimer(const std::string& name, size_t timeout, size_t frequency)
		: _timer(name)
	{
		_impl.reset(new EventWatcher(_timer, timeout, frequency));

		_impl->OnOccured.connect(OnOccured.invoker());
		_impl->OnTimeout.connect(OnTimeout.invoker());
	}


	EventWatcherWithTimer::~EventWatcherWithTimer()
	{ _impl.reset(); }


	void EventWatcherWithTimer::Touch()
	{ _impl->Touch(); }

}
