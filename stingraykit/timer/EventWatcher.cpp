// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/timer/EventWatcher.h>


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

		_tokens += _impl->OnOccured.connect(OnOccured.invoker());
		_tokens += _impl->OnTimeout.connect(OnTimeout.invoker());
	}


	EventWatcherWithTimer::~EventWatcherWithTimer()
	{ _impl.reset(); }


	void EventWatcherWithTimer::Touch()
	{ _impl->Touch(); }

}
