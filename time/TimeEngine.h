#ifndef STINGRAY_TOOLKIT_TIME_TIMEENGINE_H
#define STINGRAY_TOOLKIT_TIME_TIMEENGINE_H

#if defined(PLATFORM_POSIX)
#	include <stingray/toolkit/time/posix/TimeEngine.h>
#elif defined(PLATFORM_STAPI)
#	include <stingray/toolkit/time/osplus/TimeEngine.h>
#endif

namespace stingray
{

#if defined(PLATFORM_POSIX)
	/**
	 * @addtogroup toolkit_timer
	 * @{
	 */
	typedef posix::MonotonicTimer	MonotonicTimer;
	typedef posix::TimeEngine		TimeEngine;
	/** @} */
#elif defined(PLATFORM_STAPI)
	typedef osplus::MonotonicTimer	MonotonicTimer;
	typedef osplus::TimeEngine		TimeEngine;
#else
#	error Time engine not configured
#endif

}


#endif
