#ifndef GCC_BACKTRACE_H__
#define GCC_BACKTRACE_H__

#include <string>
#include <vector>

#include <stingray/toolkit/collection/array.h>

namespace stingray {
namespace gcc
{

	class Backtrace
	{
	public:
		static const unsigned MaxFrames = 64;

		typedef array<unsigned, MaxFrames>	BacktraceArray;

	private:
		BacktraceArray	_backtrace;
		unsigned		_size;

	public:
		Backtrace();
		std::string Get() const;
		std::string ToString() const { return Get(); }
	};

}}

#endif
