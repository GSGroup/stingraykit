#include <stingray/toolkit/locale/LangCode.h>

#include <stingray/toolkit/string/StringUtils.h>
#include <stingray/toolkit/collection/array.h>
#include <stingray/toolkit/exception.h>

namespace stingray
{
	LangCode::LangCode(u32 code) : _code(code)
	{
		if (code & 0xff000000u)
			TOOLKIT_THROW(std::string("invalid language code: ") + stingray::ToString(code));
		ToUpper();
	}

	LangCode::LangCode(const std::string &code)
	{
		if (code.size() != 3)
			TOOLKIT_THROW("invalid language code: " + code);

		char a = DoToUpper(code[0]), b = DoToUpper(code[1]), c = DoToUpper(code[2]);
		//if (a < 'A' || a > 'Z' || b < 'A' || b > 'Z' || c < 'A' || c > 'Z')
		//	TOOLKIT_THROW("invalid language code: " + code);

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
}
