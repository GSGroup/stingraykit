// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/string/Unicode.h>

#include <stingraykit/string/ToString.h>
#include <stingraykit/Mapper.h>

#ifdef HAVE_ICU_I18N
#	include <unicode/unistr.h>
#endif

namespace stingray
{

#ifdef HAVE_ICU_I18N

	namespace
	{

		struct CollationResultMapper : public BaseValueMapper<CollationResultMapper, UCollationResult, int>
		{
			using MappingsList = TypeList<
					Src::Value<UCOL_EQUAL>,		Dst::Value<0>,
					Src::Value<UCOL_LESS>,		Dst::Value<-1>,
					Src::Value<UCOL_GREATER>,	Dst::Value<1>
			>;

			using DefaultMapping = TypeList<Src::Fail, Dst::Fail>;
		};

		const icu::UnicodeString Rules = "&е < ё <<< Ё";

	}

	UnicodeCollator::UnicodeCollator(bool caseSensitive)
	{
		UErrorCode success = U_ZERO_ERROR;
		_collator = make_unique_ptr<icu::RuleBasedCollator>(Rules, success);
		STINGRAYKIT_CHECK(success != U_FILE_ACCESS_ERROR, "file requested by ICU was not found, please install icudt53l.dat to /usr/share/icu");
		STINGRAYKIT_CHECK(U_SUCCESS(success), StringBuilder() % "Can't create collator, error: " % success);

		_collator->setStrength(caseSensitive ? icu::Collator::TERTIARY : icu::Collator::SECONDARY);
	}


	UnicodeCollator::~UnicodeCollator()
	{ }


	int UnicodeCollator::Compare(const std::string& str1, const std::string& str2) const
	{
		UErrorCode success = U_ZERO_ERROR;
		const UCollationResult r = _collator->compareUTF8(str1, str2, success);
		STINGRAYKIT_CHECK(U_SUCCESS(success), StringBuilder() % "compareUTF8() failed, error: " % success);

		return CollationResultMapper::Map(r);
	}


	std::string Utf8ToLower(const std::string& str)
	{
		const icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(str).toLower();
		std::string r;
		ustr.toUTF8String(r);
		return r;
	}

#else

	UnicodeCollator::UnicodeCollator(bool caseSensitive) : _caseSensitive(caseSensitive) { }


	UnicodeCollator::~UnicodeCollator() { }


	int UnicodeCollator::Compare(const std::string& str1, const std::string& str2) const
	{ return _caseSensitive ? str1.compare(str2) : Utf8ToLower(str1).compare(Utf8ToLower(str2)); }


	std::string Utf8ToLower(const std::string& str)
	{
		std::string result;
		result.reserve(str.size());
		for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
		{
			if (*it >= 'A' && *it <= 'Z')
				result.push_back(*it + 'a' - 'A');
			else if ((u8)*it == 0xD0)
			{
				u16 c0 = (u8)*it & 0x1F;
				STINGRAYKIT_CHECK(++it != str.end(), "Malformed utf-8!");
				u16 c = (c0 << 6) | ((u8)*it & 0x3F);
				if (c >= 0x410 && c <= 0x42F)
					c += 0x20;
				else if (c >= 0x401 && c <= 0x40f)
					c += 0x50;
				result.push_back(0xD0 | (c >> 6));
				result.push_back((c & 0x3F) | 0x80);
			}
			else
				result.push_back(*it);
		}
		return result;
	}

#endif

}
