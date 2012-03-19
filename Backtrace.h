#ifndef DVRLIB_BACKTRACE_H__
#define DVRLIB_BACKTRACE_H__

#include <string>
#ifdef HAVE_BACKTRACE
#	include <dvrlib/platform/posix/threads/Backtrace.h>
#endif

namespace dvrlib
{
#ifdef HAVE_BACKTRACE
		typedef posix::Backtrace Backtrace;
#else
#	error "no backtrace configured"
#endif

}

#endif
