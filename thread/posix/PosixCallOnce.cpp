#include <stingray/toolkit/thread/posix/PosixCallOnce.h>

#include <stingray/toolkit/SystemException.h>
#include <stingray/toolkit/exception.h>

namespace stingray {


	void PosixCallOnce::CallOnce(OnceNativeType& once, void (*func)())
	{
		STINGRAY_ANNOTATE_HAPPENS_BEFORE(&once);
		if (pthread_once(&once, func))
			TOOLKIT_THROW(SystemException("pthread_once"));
		STINGRAY_ANNOTATE_HAPPENS_AFTER(&once);
	}


}


