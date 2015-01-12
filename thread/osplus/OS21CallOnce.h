#ifndef STINGRAY_TOOLKIT_THREAD_OSPLUS_OS21CALLONCE_H
#define STINGRAY_TOOLKIT_THREAD_OSPLUS_OS21CALLONCE_H


#include <string>


namespace stingray {


	struct OS21CallOnce
	{
		typedef __gthread_once_t	OnceNativeType;

		static void CallOnce(OnceNativeType& once, void (*func)());
	};

#define STINGRAYKIT_ONCE_INIT_VALUE __GTHREAD_ONCE_INIT


}


#endif

