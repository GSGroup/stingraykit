// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/locale/LangCode.h>

#include <stingraykit/string/ToString.h>

namespace stingray
{

	namespace
	{

		char DoToUpper(char c)
		{ return c >= 'a' && c <= 'z'? c - 'a' + 'A' : c; }


		char DoToLower(char c)
		{ return c >= 'A' && c <= 'Z' ? c - 'A' + 'a' : c; }


		u32 ToUpper(u32 code)
		{
			if (code == 0)
				return code;

			array<char, 3> c;
			c[0] = (code >> 16) & 0xff;
			c[1] = (code >> 8) & 0xff;
			c[2] = (code >> 0) & 0xff;

			array<char, 3> r;
			std::transform(c.begin(), c.end(), r.begin(), &DoToUpper);

			return (u32)(r[0] << 16) + (r[1] << 8) + r[2];
		}

	}


	const LangCode::AnyType LangCode::Any = { };

	LangCode LangCode::Eng() { return LangCode("eng"); }
	LangCode LangCode::Rus() { return LangCode("rus"); }


	LangCode::LangCode(u32 code) : _code(0)
	{
		STINGRAYKIT_CHECK(!(code & 0xff000000u), StringBuilder() % "Invalid language code: " % code);
		_code = ToUpper(code);
	}


	LangCode::LangCode(string_view code)
	{
		STINGRAYKIT_CHECK(code.size() == 3, StringBuilder() % "Invalid language code: " % code);

		const char a = DoToUpper(code[0]);
		const char b = DoToUpper(code[1]);
		const char c = DoToUpper(code[2]);

		//STINGRAYKIT_CHECK(a >= 'A' && a <= 'Z' && b >= 'A' && b <= 'Z' && c >= 'A' && c <= 'Z', StringBuilder() % "Invalid language code: " % code);

		_code = (((u8)a) << 16) | (((u8)b) << 8) | (((u8)c) << 0);
	}


	std::string LangCode::ToString() const
	{
		if (_code == 0)
			return std::string();

		const char r[4] = { (char)((_code >> 16) & 0xff), (char)((_code >> 8) & 0xff), (char)((_code >> 0) & 0xff), 0 };
		return r;
	}


	std::string LangCode::To2LetterString() const
	{
		const char r[3] = { DoToLower((char)((_code >> 16) & 0xff)), DoToLower((char)((_code >> 8) & 0xff)), 0 };
		return r;
	}


	LangCode LangCode::From2Letter(string_view code)
	{
		STINGRAYKIT_CHECK(code.size() >= 2, StringBuilder() % "Invalid language code: " % code);

		std::string subcode = code.substr(0, 2).copy();
		std::transform(subcode.begin(), subcode.end(), subcode.begin(), &DoToUpper);

		if (subcode == "RU")
			return LangCode("rus");

		if (subcode == "EN")
			return LangCode("eng");

		return LangCode();
	}


	LangCode LangCode::From3Letter(string_view code)
	{ return LangCode(code); }

}
