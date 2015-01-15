#ifndef STINGRAYKIT_THREAD_POSIX_POSIXCONDITIONVARIABLE_H
#define STINGRAYKIT_THREAD_POSIX_POSIXCONDITIONVARIABLE_H

#include <pthread.h>

#include <stingraykit/thread/posix/PosixThreadEngine.h>
#include <stingraykit/thread/ICancellationToken.h>
#include <stingraykit/time/Time.h>

namespace stingray
{

	class PosixConditionVariable
	{
		STINGRAYKIT_NONCOPYABLE(PosixConditionVariable);

	private:
		mutable pthread_cond_t	_cond;

	public:
		PosixConditionVariable();
		~PosixConditionVariable();

		void Wait(PosixMutex& mutex, const ICancellationToken& token);
		bool TimedWait(PosixMutex& mutex, TimeDuration interval, const ICancellationToken& token);

		void Wait(const PosixMutex& mutex) const;
		bool TimedWait(const PosixMutex& mutex, TimeDuration interval) const;

		void Broadcast();
	};

}


#endif
