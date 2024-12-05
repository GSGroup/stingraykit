#ifndef STINGRAYKIT_STRING_TRANSLATEDSTRINGUTILS_H
#define STINGRAYKIT_STRING_TRANSLATEDSTRINGUTILS_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/ForEach.h>
#include <stingraykit/string/StringFormat.h>
#include <stingraykit/string/TranslatedString.h>

#include <map>

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


	class TranslatedStringBuilder
	{
		using Builders = std::map<LangCode, StringBuilder>;

	private:
		Builders		_builders;

	public:
		template < typename... LangCodes >
		TranslatedStringBuilder(LangCode lang = LangCode::Any, LangCodes... langs)
			:	TranslatedStringBuilder(std::initializer_list<LangCode>{ lang, langs... })
		{ }

		template < typename LangCodes, decltype(std::declval<LangCodes>().begin(), std::declval<LangCodes>().end(), bool()) = false >
		TranslatedStringBuilder(const LangCodes& langs)
		{
			for (const auto& lang : langs)
				_builders.emplace(std::piecewise_construct, std::forward_as_tuple(lang), std::make_tuple());
		}

		template < typename T >
		TranslatedStringBuilder& operator % (const T& arg)
		{
			for (auto& entry : _builders)
				entry.second % Detail::GetTranslatedValue(entry.first, arg);

			return *this;
		}

		TranslatedString Get() const
		{
			TranslatedString::Builder b;
			for (const auto& entry : _builders)
				b.AddTranslation(entry.first, entry.second);

			return b.Get();
		}

		operator TranslatedString () const
		{ return Get(); }
	};


	template < typename LangCodes, typename Range >
	auto Join(const LangCodes& langs, string_view separator, const Range& range)
			-> decltype(std::declval<LangCodes>().begin(), std::declval<LangCodes>().end(), TranslatedString())
	{
		bool empty = true;
		TranslatedStringBuilder tsb(langs);
		for (const auto& item : range)
		{
			if (!empty)
				tsb % separator;
			tsb % item;
			empty = false;
		}
		return tsb;
	}

}

#endif
