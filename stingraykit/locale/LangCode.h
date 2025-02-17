#ifndef STINGRAYKIT_LOCALE_LANGCODE_H
#define STINGRAYKIT_LOCALE_LANGCODE_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/string/string_view.h>
#include <stingraykit/operators.h>
#include <stingraykit/Types.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_locale
	 * @{
	 */

	class LangCode
	{
	private:
		u32 _code;

	public:
		struct AnyType { };
		static const AnyType Any;

		static LangCode Eng();
		static LangCode Rus();

		LangCode() : _code(0) { }
		LangCode(AnyType) : _code(0) { }

		explicit LangCode(u32 code);
		explicit LangCode(string_view code);

		bool IsKnown() const;

		bool operator < (const LangCode& other) const
		{ return _code < other._code; }
		STINGRAYKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(LangCode);

		std::string ToString() const;
		std::string To2LetterString() const;

		static LangCode FromString(string_view code)
		{ return code.size() == 2 ? From2Letter(code) : From3Letter(code); }

		static LangCode From2Letter(string_view code);
		static LangCode From3Letter(string_view code);
	};

	/** @} */

}

#endif
