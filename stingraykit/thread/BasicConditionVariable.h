#ifndef STINGRAYKIT_THREAD_BASICCONDITIONVARIABLE_H
#define STINGRAYKIT_THREAD_BASICCONDITIONVARIABLE_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/time/Time.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_threads
	 * @{
	 */

	template < typename MutexType, typename SemaphoreType >
	class BasicConditionVariable
	{
		STINGRAYKIT_NONCOPYABLE(BasicConditionVariable);

		typedef GenericMutexLock<MutexType>		MutexLockType;
		typedef GenericMutexUnlock<MutexType>	MutexUnlockType;

		MutexType		_waitersMutex;
		mutable u32		_waitersCount;
		SemaphoreType	_waiter;

	public:
		BasicConditionVariable() : _waitersCount(0)
		{ }

		~BasicConditionVariable()
		{ }

		void Wait(const MutexType& mutex) const
		{
			{
				MutexLockType l(_waitersMutex);
				++_waitersCount;
			}

			MutexUnlockType u(mutex);
			_waiter.Wait();
		}

		bool TimedWait(const MutexType& mutex, TimeDuration interval) const
		{
			{
				MutexLockType l(_waitersMutex);
				++_waitersCount;
			}

			MutexUnlockType u(mutex);
			return _waiter.TimedWait(interval);
		}

		bool TimedWait(const MutexType& mutex, const Time& absTime) const
		{
			return TimedWait(mutex, absTime - Time::Now());
		}

		void Signal()
		{
			MutexLockType l(_waitersMutex);

			if (_waitersCount)
			{
				_waiter.Signal();
				--_waitersCount;
			}
		}

		void Broadcast()
		{
			MutexLockType l(_waitersMutex);

			for (; _waitersCount; --_waitersCount)
				_waiter.Signal();
		}
	};

	/** @} */


}


#endif
