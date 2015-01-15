#include <stingraykit/thread/posix/PosixCallOnce.h>

#include <stingraykit/SystemException.h>
#include <stingraykit/exception.h>

namespace stingray {


	void PosixCallOnce::CallOnce(OnceNativeType& once, void (*func)())
	{
		STINGRAYKIT_ANNOTATE_HAPPENS_BEFORE(&once);
		if (pthread_once(&once, func))
			STINGRAYKIT_THROW(SystemException("pthread_once"));
		STINGRAYKIT_ANNOTATE_HAPPENS_AFTER(&once);
	}


}


