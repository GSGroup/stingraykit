// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/posix/PosixSemaphore.h>
#include <errno.h>

#include <stingraykit/SystemException.h>
#include <stingraykit/time/posix/utils.h>

namespace stingray {
namespace posix {


	PosixSemaphore::PosixSemaphore(int count)
	{
		if(sem_init(&_semaphore, 0, count) < 0)
			STINGRAYKIT_THROW(SystemException("sem_init"));
	}

	PosixSemaphore::~PosixSemaphore()
	{ sem_destroy(&_semaphore); }

	void PosixSemaphore::Wait() const
	{
		if (sem_wait(&_semaphore) != 0)
			STINGRAYKIT_THROW(SystemException("sem_wait"));
	}

	bool PosixSemaphore::TimedWait(TimeDuration interval) const
	{
		timespec t = { };
		posix::timespec_now(CLOCK_REALTIME, &t);
		posix::timespec_add(&t, interval);

		int result = sem_timedwait(&_semaphore, &t);
		if (result != 0 && errno != ETIMEDOUT)
			STINGRAYKIT_THROW(SystemException("sem_timedwait"));
		else
			return result == 0;
	}

	bool PosixSemaphore::TimedWait(Time absTime) const
	{
		timespec t = { };
		posix::timespec_add(&t, TimeDuration::FromMilliseconds(absTime.GetMilliseconds()));

		int result = sem_timedwait(&_semaphore, &t);
		if (result != 0 && errno != ETIMEDOUT)
			STINGRAYKIT_THROW(SystemException("sem_timedwait"));
		else
			return result == 0;
	}

	void PosixSemaphore::Signal() const
	{
		if (sem_post(&_semaphore))
			STINGRAYKIT_THROW(SystemException("sem_post"));
	}


}
}


