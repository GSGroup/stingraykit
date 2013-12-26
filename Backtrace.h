#ifndef STINGRAY_TOOLKIT_BACKTRACE_H
#define STINGRAY_TOOLKIT_BACKTRACE_H

#include <string>
#if defined(HAVE_BACKTRACE)
#	include <stingray/platform/posix/threads/Backtrace.h>
#elif defined(USE_GCC_BACKTRACE)
#	include <stingray/platform/gcc/Backtrace.h>
#endif

namespace stingray
{
#if defined(HAVE_BACKTRACE)
	typedef posix::Backtrace Backtrace;
#elif defined(USE_GCC_BACKTRACE)
	typedef gcc::Backtrace Backtrace;
#else
	struct Backtrace
	{
		inline std::string Get() const { return std::string(); }
		std::string ToString() const { return Get(); }
	};
#endif

}

#endif
