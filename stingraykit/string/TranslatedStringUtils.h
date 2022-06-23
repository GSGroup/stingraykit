#ifndef STINGRAYKIT_STRING_TRANSLATEDSTRINGUTILS_H
#define STINGRAYKIT_STRING_TRANSLATEDSTRINGUTILS_H

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
	auto Join(const LangCodes& langs, const std::string& separator, const Range& range)
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
