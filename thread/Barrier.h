#ifndef STINGRAY_TOOLKIT_THREAD_BARRIER_H
#define STINGRAY_TOOLKIT_THREAD_BARRIER_H

#include <stingray/toolkit/thread/ConditionVariable.h>
#include <stingray/toolkit/thread/Thread.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_threads
	 * @{
	 */

	class Barrier
	{
	private:
		const size_t		_requiredCount;
		size_t				_currentCount;
		bool				_releasing;
		Mutex				_mutex;
		ConditionVariable	_cv;

	public:
		Barrier(size_t count) : _requiredCount(count), _currentCount(0), _releasing(false)
		{}

		~Barrier()
		{}

		bool Wait()
		{
			MutexLock l(_mutex);
			while (_releasing)
				_cv.Wait(_mutex);
			if (++_currentCount == _requiredCount)
			{
				_releasing = true;
				_cv.Broadcast();
			}
			else
				_cv.Wait(_mutex);
			if (--_currentCount == 0)
			{
				_releasing = false;
				_cv.Broadcast();
				return true;
			}
			return false;
		}
	};

	/** @} */

}


#endif

