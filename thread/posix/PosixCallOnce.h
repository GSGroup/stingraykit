#ifndef STINGRAY_TOOLKIT_THREAD_POSIX_POSIXCALLONCE_H
#define STINGRAY_TOOLKIT_THREAD_POSIX_POSIXCALLONCE_H

#include <pthread.h>

namespace stingray {


	struct PosixCallOnce
	{
		typedef pthread_once_t		OnceNativeType;

		static void CallOnce(OnceNativeType& once, void (*func)());
	};

#define STINGRAY_ONCE_INIT_VALUE PTHREAD_ONCE_INIT


}


#endif

