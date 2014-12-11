#include <stingray/toolkit/log/Logger.h>
#include <stingray/toolkit/Backtrace.h>


extern "C" typeof(abort) __real_abort;

extern "C" void __wrap_abort(ssize_t size)
{
	using namespace stingray;

	Logger::Error() << "Abort called: " << Backtrace().Get();

	__real_abort();
}
