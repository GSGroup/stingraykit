#ifndef STINGRAYKIT_STRING_TRANSLATEDSTRING_H
#define STINGRAYKIT_STRING_TRANSLATEDSTRING_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/IDictionary.h>
#include <stingraykit/locale/LangCode.h>
#include <stingraykit/serialization/ISerializable.h>

namespace stingray
{

	struct TranslatedString
	{
	private:
		typedef IDictionary<LangCode, std::string>	DictionaryImpl;
		STINGRAYKIT_DECLARE_PTR(DictionaryImpl);

	public:
		typedef IReadonlyDictionary<LangCode, std::string>	Dictionary;
		STINGRAYKIT_DECLARE_PTR(Dictionary);

		class Builder;

	private:
		DictionaryImplPtr	_dictionary;

		explicit TranslatedString(const DictionaryImplPtr& dictionary);

	public:
		TranslatedString();
		TranslatedString(LangCode lang, const std::string& str);

		bool HasTranslation(LangCode lang) const;
		std::string GetTranslation(LangCode lang) const;
		optional<std::string> TryGetTranslation(LangCode lang) const;

		DictionaryPtr GetTranslations() const
		{ return _dictionary; }

		bool IsEmpty() const
		{ return _dictionary->IsEmpty(); }

		void Serialize(ObjectOStream& ar) const;
		void Deserialize(ObjectIStream& ar);

		std::string ToString() const;

		template < typename... LangCodes >
		std::string SelectTranslation(LangCode lang, LangCodes... langCodes) const
		{ return SelectTranslation(std::initializer_list<LangCode>{ lang, langCodes..., LangCode::Any }); }

		template < typename LangCodes >
		auto SelectTranslation(const LangCodes& langCodes) const -> decltype(langCodes.begin(), langCodes.end(), std::string())
		{
			for (const auto& lang : langCodes)
				if (const optional<std::string> result = TryGetTranslation(lang))
					return *result;

			return "";
		}

		int Compare(const TranslatedString& other) const;
		STINGRAYKIT_GENERATE_COMPARISON_OPERATORS_FROM_COMPARE(TranslatedString);
	};


	class TranslatedString::Builder
	{
		STINGRAYKIT_NONCOPYABLE(Builder);

	private:
		DictionaryImplPtr	_product;
		bool				_dirty;

	public:
		Builder();
		explicit Builder(const TranslatedString& other);

		Builder& AddTranslation(LangCode lang, const std::string& str);

		operator TranslatedString ()
		{ return Get(); }

		TranslatedString Get();
	};

}

#endif

