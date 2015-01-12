#include <stingray/toolkit/thread/osplus/OS21CallOnce.h>

#include <stingray/toolkit/exception.h>

namespace stingray {


	void OS21CallOnce::CallOnce(OnceNativeType& once, void (*func)())
	{ STINGRAYKIT_CHECK( !__gthread_once(&once, func), std::runtime_error("__gthread_once") ); }


}


