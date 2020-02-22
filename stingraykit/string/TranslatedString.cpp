// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/string/TranslatedString.h>

#include <stingraykit/collection/CollectionBuilder.h>
#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/MapDictionary.h>
#include <stingraykit/serialization/Serialization.h>

namespace stingray
{

	TranslatedString::TranslatedString(const DictionaryImplPtr& dictionary)
		: _dictionary(STINGRAYKIT_REQUIRE_NOT_NULL(dictionary))
	{ }


	TranslatedString::TranslatedString()
		: _dictionary(make_shared_ptr<MapDictionary<LangCode, std::string> >())
	{ }


	TranslatedString::TranslatedString(LangCode lang, const std::string& str)
		: _dictionary(make_shared_ptr<MapDictionary<LangCode, std::string> >())
	{ _dictionary->Set(lang, str); }


	bool TranslatedString::HasTranslation(LangCode lang) const
	{
		if (lang == LangCode::Any)
			return !_dictionary->IsEmpty();

		return _dictionary->ContainsKey(lang);
	}


	std::string TranslatedString::GetTranslation(LangCode lang) const
	{
		if (lang == LangCode::Any)
			return _dictionary->GetEnumerator()->Get().Value;

		return _dictionary->Get(lang);
	}


	void TranslatedString::Serialize(ObjectOStream& ar) const
	{ ar.Serialize("translations", *_dictionary); }


	void TranslatedString::Deserialize(ObjectIStream& ar)
	{ ar.Deserialize("translations", *_dictionary); }


	std::string TranslatedString::ToString() const
	{ return stingray::ToString(_dictionary); }


	std::string TranslatedString::SelectTranslation(LangCode l0) const
	{ return DoSelectTranslation(VectorBuilder<LangCode>(l0, LangCode::Any)); }


	std::string TranslatedString::SelectTranslation(LangCode l0, LangCode l1) const
	{ return DoSelectTranslation(VectorBuilder<LangCode>(l0, l1, LangCode::Any)); }


	std::string TranslatedString::SelectTranslation(LangCode l0, LangCode l1, LangCode l2) const
	{ return DoSelectTranslation(VectorBuilder<LangCode>(l0, l1, l2, LangCode::Any)); }


	std::string TranslatedString::SelectTranslation(const std::vector<LangCode>& langCodes) const
	{
		for (std::vector<LangCode>::const_iterator it = langCodes.begin(); it != langCodes.end(); ++it)
			if (HasTranslation(*it))
				return GetTranslation(*it);

		return GetTranslation(LangCode::Any);
	}


	std::string TranslatedString::DoSelectTranslation(const std::vector<LangCode>& langCodes) const
	{
		for (std::vector<LangCode>::const_iterator it = langCodes.begin(); it != langCodes.end(); ++it)
			if (HasTranslation(*it))
				return GetTranslation(*it);
		return "";
	}


	int TranslatedString::DoCompare(const TranslatedString& other) const
	{ return Enumerable::MakeSequenceCmp(comparers::Cmp())(_dictionary, other._dictionary); }


	TranslatedString::Builder::Builder()
		: _product(make_shared_ptr<MapDictionary<LangCode, std::string> >()), _dirty(true)
	{ }


	TranslatedString::Builder::Builder(const TranslatedString& other)
		: _product(other._dictionary), _dirty(false)
	{ }


	TranslatedString::Builder& TranslatedString::Builder::AddTranslation(LangCode lang, const std::string& str)
	{
		if (!_dirty)
		{
			_product = make_shared_ptr<MapDictionary<LangCode, std::string> >(_product);
			_dirty = true;
		}

		_product->Set(lang, str);
		return *this;
	}


	TranslatedString TranslatedString::Builder::Get()
	{
		DictionaryImplPtr result;
		result.swap(_product);
		return TranslatedString(result);
	}

}
