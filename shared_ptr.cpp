#include <stingray/toolkit/shared_ptr.h>

#include <stingray/log/Logger.h>
#include <stingray/toolkit/Backtrace.h>


namespace stingray
{
	namespace Detail
	{
		void DoLogAddRef(const char* className, atomic_int_type refs, const void* objPtrVal, const void* sharedPtrPtrVal)
		{ Logger::Warning() << "[shared_ptr] +++ Addrefed " << className << " (obj: 0x" << Hex(objPtrVal, 8) << ", shared_ptr: 0x" << Hex(sharedPtrPtrVal, 8) << "), new value = " << refs << "\n" << Backtrace().Get(); }

		void DoLogReleaseRef(const char* className, atomic_int_type refs, const void* objPtrVal, const void* sharedPtrPtrVal)
		{ Logger::Warning() << "[shared_ptr] --- Released " << className << " (obj: 0x" << Hex(objPtrVal, 8) << ", shared_ptr: 0x" << Hex(sharedPtrPtrVal, 8) << "), new value = " << refs << "\n" << Backtrace().Get(); }
	}
}
