#ifndef STINGRAYKIT_TIMER_EVENTWATCHER_H
#define STINGRAYKIT_TIMER_EVENTWATCHER_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/timer/Timer.h>
#include <stingraykit/signal/signals.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_profiling
	 * @{
	 */

	class EventWatcher
	{
	private:
		ExecutionDeferrerPtr	_deferrer;
		size_t					_frequency;
		size_t					_count;

	public:
		EventWatcher(Timer& timer, size_t timeout, size_t frequency);
		~EventWatcher();

		void Touch();

		signal<void ()> OnOccured;
		signal<void ()> OnTimeout;

	private:
		void Timeout();
	};
	STINGRAYKIT_DECLARE_PTR(EventWatcher);


	class EventWatcherWithTimer
	{
	private:
		Timer 				_timer;
		EventWatcherPtr		_impl;
		TokenPool			_tokens;

	public:
		EventWatcherWithTimer(const std::string& name, size_t timeout, size_t frequency);
		~EventWatcherWithTimer();

		void Touch();

		signal<void ()> OnOccured;
		signal<void ()> OnTimeout;
	};
	STINGRAYKIT_DECLARE_PTR(EventWatcherWithTimer);

	/** @} */

}


#endif
