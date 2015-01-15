#ifndef STINGRAYKIT_TIMER_EVENTWATCHER_H
#define STINGRAYKIT_TIMER_EVENTWATCHER_H


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
