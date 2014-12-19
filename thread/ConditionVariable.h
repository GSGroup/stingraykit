#ifndef STINGRAY_TOOLKIT_THREAD_CONDITIONVARIABLE_H
#define STINGRAY_TOOLKIT_THREAD_CONDITIONVARIABLE_H


#include <stingray/toolkit/thread/Thread.h>

#if PROFILE_MUTEX_LOCK_TIMINGS
#	include <stingray/toolkit/thread/BasicConditionVariable.h>
namespace stingray { typedef BasicConditionVariable<Mutex, Semaphore>	ConditionVariable; }
#elif PLATFORM_POSIX
#	include <stingray/toolkit/thread/posix/PosixConditionVariable.h>
namespace stingray { typedef PosixConditionVariable						ConditionVariable; }
#else
#	include <stingray/toolkit/thread/BasicConditionVariable.h>
namespace stingray { typedef BasicConditionVariable<Mutex, Semaphore>	ConditionVariable; }
#endif


#endif
