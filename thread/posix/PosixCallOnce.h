#ifndef STINGRAYKIT_THREAD_POSIX_POSIXCALLONCE_H
#define STINGRAYKIT_THREAD_POSIX_POSIXCALLONCE_H

#include <pthread.h>

namespace stingray {


	struct PosixCallOnce
	{
		typedef pthread_once_t		OnceNativeType;

		static void CallOnce(OnceNativeType& once, void (*func)());
	};

#define STINGRAYKIT_ONCE_INIT_VALUE PTHREAD_ONCE_INIT


}


#endif

