#ifndef STINGRAYKIT_STRING_TRANSLATEDSTRING_H
#define STINGRAYKIT_STRING_TRANSLATEDSTRING_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
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
	public:
		typedef IDictionary<LangCode, std::string>	Dictionary;
		typedef shared_ptr<Dictionary> DictionaryPtr;

	private:
		DictionaryPtr	_dictionary;

	public:
		TranslatedString();
		~TranslatedString();

		TranslatedString& AddTranslation(LangCode lang, const std::string& str);

		bool HasTranslation(LangCode lang) const;
		std::string GetTranslation(LangCode lang) const;

		DictionaryPtr GetTranslations() const;

		bool IsEmpty() const
		{ return _dictionary->IsEmpty(); }

		void Serialize(ObjectOStream &ar) const;
		void Deserialize(ObjectIStream &ar);

		std::string ToString() const;

		std::string SelectTranslation(LangCode l0) const;
		std::string SelectTranslation(LangCode l0, LangCode l1) const;
		std::string SelectTranslation(LangCode l0, LangCode l1, LangCode l2) const;
		std::string SelectTranslation(const std::vector<LangCode>& langCodes) const;

		bool operator < (const TranslatedString& other) const
		{ return DoCompare(other) < 0; }
		STINGRAYKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(TranslatedString);

	private:
		std::string DoSelectTranslation(const std::vector<LangCode>& langCodes) const;
		int DoCompare(const TranslatedString& other) const;
	};

}

#endif

