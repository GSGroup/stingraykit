// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/toolkit.h>

#include <stingraykit/function/bind.h>
#include <stingraykit/log/Logger.h>
#include <stingraykit/Holder.h>

#if defined(__GNUC__) || defined(__clang__)
#	include <cxxabi.h>
#endif

namespace stingray
{

	std::string ToolkitWhere::ToString() const
	{ return StringBuilder() % _functionName % " (" % _file % ":" % _line % ")"; }


	namespace Detail
	{
		void ArrayCheckRange(size_t pos, size_t size)
		{ STINGRAYKIT_CHECK_RANGE(pos, size); }
	}


	void DebuggingHelper::BreakpointHere()
	{ }


	void DebuggingHelper::TerminateWithMessage(ToolkitWhere where, const std::string& message) noexcept
	{
		Logger::Error() << "Terminate was requested from " << where << "\nbacktrace: " << Backtrace() << "\n" << message;
		std::terminate();
	}


#if !defined(__GNUC__) && !defined(__clang__)
	std::string Demangle(const std::string& s)
	{ return s; }
#else
	std::string Demangle(const std::string& s)
	{
		int status = 0;
		ScopedHolder<void*> holder(&free);

		char* result = abi::__cxa_demangle(s.c_str(), 0, 0, &status);
		if (result)
			holder.Set(result);

		return status != 0 ? s : result;
	}
#endif

}
