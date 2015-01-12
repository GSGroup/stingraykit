#ifndef STINGRAY_TOOLKIT_THREAD_CALL_ONCE_H
#define STINGRAY_TOOLKIT_THREAD_CALL_ONCE_H

#if PLATFORM_POSIX
#	include <stingray/toolkit/thread/posix/PosixCallOnce.h>
	namespace stingray { typedef PosixCallOnce	CallOnce; }
#elif PLATFORM_OSPLUS
#	include <stingray/toolkit/thread/osplus/OS21CallOnce.h>
	namespace stingray { typedef OS21CallOnce	CallOnce; }
#else
#	error Threads not configured
#endif

namespace stingray
{

	/**
	 * @addtogroup toolkit_threads
	 * @{
	 */

#define STINGRAYKIT_DECLARE_ONCE_FLAG(flagName_) ::stingray::CallOnce::OnceNativeType flagName_
#define STINGRAYKIT_DEFINE_ONCE_FLAG(flagName_) ::stingray::CallOnce::OnceNativeType flagName_ = STINGRAYKIT_ONCE_INIT_VALUE

	inline void call_once(CallOnce::OnceNativeType& flag, void (*func)())
	{ CallOnce::CallOnce(flag, func); }

	/** @} */

}


#endif
