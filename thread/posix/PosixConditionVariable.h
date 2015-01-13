#ifndef STINGRAYKIT_THREAD_POSIX_POSIXCONDITIONVARIABLE_H
#define STINGRAYKIT_THREAD_POSIX_POSIXCONDITIONVARIABLE_H

#include <pthread.h>

#include <stingray/toolkit/thread/posix/PosixThreadEngine.h>
#include <stingray/toolkit/thread/ICancellationToken.h>
#include <stingray/toolkit/time/Time.h>

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
