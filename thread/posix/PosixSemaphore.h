#ifndef STINGRAY_TOOLKIT_THREAD_POSIX_POSIXSEMAPHORE_H
#define STINGRAY_TOOLKIT_THREAD_POSIX_POSIXSEMAPHORE_H

#include <pthread.h>
#include <semaphore.h>

#include <stingray/toolkit/time/Time.h>
#include <stingray/toolkit/exception.h>

namespace stingray {
namespace posix {


	class PosixSemaphore
	{
		STINGRAYKIT_NONCOPYABLE(PosixSemaphore);

	private:
		mutable sem_t	_semaphore;

	public:
		PosixSemaphore(int count = 0);
		~PosixSemaphore();

		void Signal() const;
		void Signal(int n) const { while(n-- > 0) Signal(); }

		void Wait() const;

		bool TimedWait(TimeDuration interval) const;
		bool TimedWait(const Time& absTime) const;
	};


}
}


#endif

