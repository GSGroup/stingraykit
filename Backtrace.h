#ifndef STINGRAY_TOOLKIT_BACKTRACE_H
#define STINGRAY_TOOLKIT_BACKTRACE_H

#include <string>
#ifdef HAVE_BACKTRACE
#	include <stingray/platform/posix/threads/Backtrace.h>
#endif

namespace stingray
{
#ifdef HAVE_BACKTRACE
	typedef posix::Backtrace Backtrace;
#else
	struct Backtrace
	{
		inline std::string Get() const { return std::string(); }
	};
#endif

}

#endif
