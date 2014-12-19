#include <stingray/toolkit/thread/posix/PosixSemaphore.h>
#include <errno.h>

#include <stingray/toolkit/SystemException.h>
#include <stingray/toolkit/time/posix/utils.h>

namespace stingray {
namespace posix {


	PosixSemaphore::PosixSemaphore(int count)
	{
		if(sem_init(&_semaphore, 0, count) < 0)
			TOOLKIT_THROW(SystemException("sem_init"));
	}

	PosixSemaphore::~PosixSemaphore()
	{ sem_destroy(&_semaphore); }

	void PosixSemaphore::Wait() const
	{
		if (sem_wait(&_semaphore) != 0)
			TOOLKIT_THROW(SystemException("sem_wait"));
	}

	bool PosixSemaphore::TimedWait(TimeDuration interval) const
	{
		timespec t = { };
		posix::timespec_now(CLOCK_REALTIME, &t);
		posix::timespec_add(&t, interval);

		int result = sem_timedwait(&_semaphore, &t);
		if (result != 0 && errno != ETIMEDOUT)
			TOOLKIT_THROW(SystemException("sem_timedwait"));
		else
			return result == 0;
	}

	bool PosixSemaphore::TimedWait(const Time& absTime) const
	{
		timespec t = { };
		posix::timespec_add(&t, TimeDuration::FromMilliseconds(absTime.GetMilliseconds()));

		int result = sem_timedwait(&_semaphore, &t);
		if (result != 0 && errno != ETIMEDOUT)
			TOOLKIT_THROW(SystemException("sem_timedwait"));
		else
			return result == 0;
	}

	void PosixSemaphore::Signal() const
	{
		if (sem_post(&_semaphore))
			TOOLKIT_THROW(SystemException("sem_post"));
	}


}
}


