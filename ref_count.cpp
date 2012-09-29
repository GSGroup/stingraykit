#include <stingray/toolkit/ref_count.h>

#include <stingray/log/Logger.h>


namespace stingray
{
	namespace Detail
	{
		void DoLogAddRef(const char* className, atomic_int_type refs, const void* ptrVal)
		{ Logger::Warning() << "[shared_ptr] +++ Addrefed " << className << " (0x" << Hex(ptrVal, 8) << "), new value = " << refs << "\n" << Backtrace().Get(); }

		void DoLogRelease(const char* className, atomic_int_type refs, const void* ptrVal)
		{ Logger::Warning() << "[shared_ptr] --- Released " << className << " (0x" << Hex(ptrVal, 8) << "), new value = " << refs << "\n" << Backtrace().Get(); }
	}
}
