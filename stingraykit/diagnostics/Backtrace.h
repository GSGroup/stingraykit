#ifndef STINGRAYKIT_DIAGNOSTICS_BACKTRACE_H
#define STINGRAYKIT_DIAGNOSTICS_BACKTRACE_H

#include <string>
#if defined(HAVE_BACKTRACE)
#	include <stingraykit/thread/posix/Backtrace.h>
#elif defined(USE_GCC_BACKTRACE)
#	include <stingraykit/diagnostics/gcc/Backtrace.h>
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
		Backtrace()						{ }
		inline std::string Get() const	{ return std::string(); }
		std::string ToString() const	{ return Get(); }
	};
#endif

}

#endif
