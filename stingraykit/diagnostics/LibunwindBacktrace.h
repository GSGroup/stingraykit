#ifndef STINGRAYKIT_DIAGNOSTICS_LIBUNWINDBACKTRACE_H
#define STINGRAYKIT_DIAGNOSTICS_LIBUNWINDBACKTRACE_H

#include <stingraykit/collection/array.h>

namespace stingray
{

	class LibunwindBacktrace
	{
	public:
		static const size_t MaxFrames = 64;

		using BacktraceArray = array<uintptr_t, MaxFrames>;

	private:
		BacktraceArray	_backtrace;
		unsigned		_size;

	public:
		LibunwindBacktrace();
		std::string Get() const;
		std::string ToString() const { return Get(); }
	};

}

#endif
