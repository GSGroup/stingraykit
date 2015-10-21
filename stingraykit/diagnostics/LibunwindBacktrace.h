#ifndef STINGRAYKIT_DIAGNOSTICS_LIBUNWINDBACKTRACE_H
#define STINGRAYKIT_DIAGNOSTICS_LIBUNWINDBACKTRACE_H

#include <string>

#include <stingraykit/collection/array.h>

namespace stingray
{

	class LibunwindBacktrace
	{
	public:
		static const unsigned MaxFrames = 64;

		typedef array<unsigned, MaxFrames>	BacktraceArray;

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
