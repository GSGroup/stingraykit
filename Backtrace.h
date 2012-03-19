#ifndef DVRLIB_BACKTRACE_H__
#define DVRLIB_BACKTRACE_H__

#include <string>
#ifdef HAVE_BACKTRACE
#	include <stingray/platform/posix/threads/Backtrace.h>
#endif

namespace stingray
{
#ifdef HAVE_BACKTRACE
		typedef posix::Backtrace Backtrace;
#else
#	error "no backtrace configured"
#endif

}

#endif
