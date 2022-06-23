#ifndef STINGRAYKIT_STRING_TRANSLATEDSTRINGUTILS_H
#define STINGRAYKIT_STRING_TRANSLATEDSTRINGUTILS_H

#include <stingraykit/collection/ForEach.h>
#include <stingraykit/string/StringFormat.h>
#include <stingraykit/string/TranslatedString.h>

namespace stingray
{

	namespace Detail
	{

		std::string GetTranslatedValue(LangCode code, const TranslatedString& arg)
		{ return arg.SelectTranslation(code); }

		template < typename T >
		const T& GetTranslatedValue(LangCode code, const T& arg)
		{ return arg; }

	}


	template < typename... Ts >
	TranslatedString TranslatedStringFormat(const TranslatedString& str, const Ts&... args)
	{
		TranslatedString::Builder b;
		FOR_EACH(const TranslatedString::Dictionary::PairType tr IN str.GetTranslations())
			b.AddTranslation(tr.Key, StringFormat(tr.Value, Detail::GetTranslatedValue(tr.Key, args)...));
		return b;
	}

}

#endif
