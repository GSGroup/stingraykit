// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/diagnostics/LibunwindBacktrace.h>

#include <stingraykit/string/Hex.h>

#include <libunwind.h>

namespace stingray
{

	LibunwindBacktrace::LibunwindBacktrace()
		: _size(0)
	{
		unw_cursor_t cursor; unw_context_t uc;
		unw_word_t ip, sp;

		unw_getcontext(&uc);
		unw_init_local(&cursor, &uc);
		while (unw_step(&cursor) > 0 && _size < _backtrace.size())
		{
			unw_get_reg(&cursor, UNW_REG_IP, &ip);
			_backtrace[_size++] = ip;
		}
	}


	std::string LibunwindBacktrace::Get() const
	{
		if (_size == 0)
			return "<empty>";

		string_ostream backtrace;

		for (size_t i = 0; i < _size; ++i)
		{
			if (i > 0)
				backtrace << " ";
			backtrace << ToHex(_backtrace[i], sizeof(uintptr_t) * 2);
		}

		return backtrace.str();
	}

}
