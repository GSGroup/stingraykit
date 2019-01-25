#ifndef STINGRAYKIT_LOCALE_LANGCODE_H
#define STINGRAYKIT_LOCALE_LANGCODE_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <string>

#include <stingraykit/toolkit.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_locale
	 * @{
	 */

	class LangCode
	{
		u32 _code;

	public:

		struct AnyType {};
		static const AnyType Any;

		static LangCode Eng();
		static LangCode Rus();

		inline LangCode() : _code(0) { }
		inline LangCode(AnyType) : _code(0) { }

		explicit LangCode(u32 code);
		LangCode(const std::string &code);
		LangCode(const char *code);

		inline bool operator <  (const LangCode& other) const	{ return _code < other._code; }
		STINGRAYKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(LangCode);

		static LangCode FromString(const std::string &code)		{ return code.length() == 2 ? From2Letter(code) : From3Letter(code); }
		std::string ToString() const;
		static LangCode From3Letter(const std::string &code);
		static LangCode From2Letter(const std::string &code);

	private:
		void ToUpper();
		static char DoToUpper(char c);
	};

	/** @} */

}


#endif
