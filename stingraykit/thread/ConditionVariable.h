#ifndef STINGRAYKIT_THREAD_CONDITIONVARIABLE_H
#define STINGRAYKIT_THREAD_CONDITIONVARIABLE_H


#include <stingraykit/thread/Thread.h>

#if PROFILE_MUTEX_LOCK_TIMINGS
#	include <stingraykit/thread/BasicConditionVariable.h>
namespace stingray { typedef BasicConditionVariable<Mutex, Semaphore>	ConditionVariable; }
#elif PLATFORM_POSIX
#	include <stingraykit/thread/posix/PosixConditionVariable.h>
namespace stingray { typedef PosixConditionVariable						ConditionVariable; }
#else
#	include <stingraykit/thread/BasicConditionVariable.h>
namespace stingray { typedef BasicConditionVariable<Mutex, Semaphore>	ConditionVariable; }
#endif


#endif
