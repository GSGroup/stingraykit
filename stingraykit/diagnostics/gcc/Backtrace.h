#ifndef GCC_BACKTRACE_H__
#define GCC_BACKTRACE_H__

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <string>

#include <stingraykit/collection/array.h>

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
