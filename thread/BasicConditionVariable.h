#ifndef STINGRAY_TOOLKIT_THREAD_BASICCONDITIONVARIABLE_H
#define STINGRAY_TOOLKIT_THREAD_BASICCONDITIONVARIABLE_H


#include <stingray/time/Time.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_threads
	 * @{
	 */

	template < typename MutexType, typename SemaphoreType >
	class BasicConditionVariable
	{
		TOOLKIT_NONCOPYABLE(BasicConditionVariable);

		typedef GenericMutexLock<MutexType>		MutexLockType;
		typedef GenericMutexUnlock<MutexType>	MutexUnlockType;

		MutexType				_waitersMutex;
		mutable atomic_int_type	_waitersCount;
		SemaphoreType			_waiter;

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
