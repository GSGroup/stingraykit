#ifndef STINGRAYKIT_LOG_NAMEDLOGGERPARAMS_H
#define STINGRAYKIT_LOG_NAMEDLOGGERPARAMS_H

// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/atomic.h>

namespace stingray
{

	class NamedLoggerParams
	{
	private:
		const char*						_name;
		atomic<bool>					_backtrace;
		atomic<bool>					_highlight;

	public:
		NamedLoggerParams(const char* name) : _name(name), _backtrace(false), _highlight(false)
		{ }

		const char* GetName() const			{ return _name; }

		bool BacktraceEnabled() const		{ return _backtrace; }
		void EnableBacktrace(bool enable)	{ _backtrace = enable; }

		bool HighlightEnabled() const		{ return _highlight; }
		void EnableHighlight(bool enable)	{ _highlight = enable; }
	};

}

#endif
