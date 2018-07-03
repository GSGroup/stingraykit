// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/locale/Translit.h>
#include <string.h>


namespace stingray
{

	static bool TranslitMatchChar(const char* translitStr, size_t translitStrSize, const char* str, size_t strSize, size_t& translitStrSkip, size_t& strSkip)
	{
		const char* translit_mapping[] = { "zh\0ж", "ch\0ч", "sh\0ш", "sch\0щ", "yu\0ю", "ya\0я", "a\0а", "b\0б", "c\0ц", "d\0д", "e\0е", "e\0э", "f\0ф", "g\0г", "h\0х", "i\0и", "j\0й", "j\0ж", "k\0ку", "l\0л", "m\0м", "n\0н", "o\0о", "p\0п", "q\0к", "r\0р", "s\0с", "t\0т", "u\0у", "v\0в", "w\0в", "x\0кс", "y\0ы", "z\0з" };

		if (strSize == 0 || translitStrSize == 0)
			return false;

		const size_t count = sizeof(translit_mapping) / sizeof(translit_mapping[0]);
		for (size_t i = 0; i < count; ++i)
		{
			const char* k = translit_mapping[i];
			const char* v = strchr(k, '\0') + 1;
			size_t kl = strlen(k);
			size_t vl = strlen(v);

			if (kl > translitStrSize || vl > strSize)
				continue;

			if (strncmp(translitStr, k, kl) == 0 && strncmp(str, v, vl) == 0)
			{
				translitStrSkip = kl;
				strSkip = vl;
				return true;
			}
		}
		return false;
	}

	bool TranslitMatchSubstr(const std::string& translitSubstr, const std::string& str) // TODO: reimplement =)
	{
		if (str.size() < translitSubstr.size())
			return false;
		
		for (size_t i = 0; i <= str.size() - translitSubstr.size(); ++i)
		{
			bool ok = true;
			for (size_t j = 0, j2 = i; j < translitSubstr.size(); ++j, ++j2)
			{
				if (str[j2] == translitSubstr[j])
					continue;
				size_t a = 1, b = 1;
				if (TranslitMatchChar(&translitSubstr[j], translitSubstr.size() - j, &str[j2], str.size() - j2, a, b))
				{
					j += a - 1;
					j2 += b - 1;
					continue;
				}
				ok = false;
				break;
			}
			if (ok)
				return true;
		}
		return false;
	}

}
