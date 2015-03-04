#ifndef STINGRAYKIT_DIAGNOSTICS_BACKTRACE_H
#define STINGRAYKIT_DIAGNOSTICS_BACKTRACE_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

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
