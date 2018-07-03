// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/locale/LangCode.h>

#include <stingraykit/string/ToString.h>
#include <stingraykit/collection/array.h>
#include <stingraykit/exception.h>

namespace stingray
{
	const LangCode::AnyType LangCode::Any = {};

	LangCode LangCode::Eng() { return LangCode("eng"); }
	LangCode LangCode::Rus() { return LangCode("rus"); }

	LangCode::LangCode(u32 code) : _code(code)
	{
		if (code & 0xff000000u)
			STINGRAYKIT_THROW(std::string("invalid language code: ") + stingray::ToString(code));
		ToUpper();
	}

	LangCode::LangCode(const std::string &code)
	{
		if (code.size() != 3)
			STINGRAYKIT_THROW("invalid language code: " + code);

		char a = DoToUpper(code[0]), b = DoToUpper(code[1]), c = DoToUpper(code[2]);
		//if (a < 'A' || a > 'Z' || b < 'A' || b > 'Z' || c < 'A' || c > 'Z')
		//	STINGRAYKIT_THROW("invalid language code: " + code);

		_code = (((u8)a) << 16) | (((u8)b) << 8) | (((u8)c) << 0);
	}

	LangCode::LangCode(const char *code)
	{ _code = LangCode(std::string(code))._code; }

	std::string LangCode::ToString() const
	{
		if (_code)
		{
			char r[4] = { (char)((_code >> 16) & 0xff), (char)((_code >> 8) & 0xff), (char)((_code >> 0) & 0xff), 0 };
			return r;
		}
		else
			return std::string();
	}

	void LangCode::ToUpper()
	{
		if (_code == 0)
			return;

		array<char, 3> c, r;
		c[0] = (_code >> 16) & 0xff;
		c[1] = (_code >> 8) & 0xff;
		c[2] = (_code >> 0) & 0xff;
		std::transform(c.begin(), c.end(), r.begin(), &LangCode::DoToUpper);
		_code = (r[0] << 16) + (r[1] << 8) + r[2];
	}

	char LangCode::DoToUpper(char c)
	{
		return c >= 'a' && c <= 'z'? c - 'a' + 'A': c;
	}

	LangCode LangCode::From2Letter(const std::string &code)
	{
		STINGRAYKIT_CHECK(code.length() >= 2, "invalid language code: " + code);

		std::string subcode(code, 0, 2);

		std::transform(subcode.begin(), subcode.end(), subcode.begin(), &LangCode::DoToUpper);

		if (subcode == "RU")
			return LangCode("rus");

		if (subcode == "EN")
			return LangCode("eng");

		return LangCode();
	}

	LangCode LangCode::From3Letter(const std::string &code)
	{
		return LangCode(code);
	}
}
