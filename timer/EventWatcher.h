#ifndef STINGRAY_TOOLKIT_TIMER_EVENTWATCHER_H
#define STINGRAY_TOOLKIT_TIMER_EVENTWATCHER_H


#include <stingray/toolkit/timer/Timer.h>
#include <stingray/toolkit/signal/signals.h>


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
