#include <stingray/toolkit/string/TranslatedString.h>

#include <map>

#include <stingray/settings/Serialization.h>
#include <stingray/toolkit/collection/CollectionBuilder.h>

namespace stingray
{


	struct TranslatedString::Impl
	{
		typedef std::map<LangCode, std::string> Container;
		Container Translations;
	};


	TranslatedString::TranslatedString() : _impl(new Impl())
	{}


	TranslatedString::TranslatedString(const TranslatedString& other) : _impl(other._impl)
	{}


	TranslatedString::~TranslatedString()
	{}


	TranslatedString& TranslatedString::operator =(const TranslatedString& other)
	{
		_impl = other._impl;
		return *this;
	}


	void TranslatedString::AddTranslation(LangCode lang, const std::string& str)
	{
		_impl->Translations[lang] = str;
	}


	bool TranslatedString::HasTranslation(LangCode lang) const
	{
		if (lang == LangCode::Any)
			return !_impl->Translations.empty();
		return _impl->Translations.find(lang) != _impl->Translations.end();
	}


	std::string TranslatedString::GetTranslation(LangCode lang) const
	{
		Impl::Container::const_iterator it = _impl->Translations.find(lang);
		if (it != _impl->Translations.end())
			return it->second;

		if (lang == LangCode::Any)
		{
			TOOLKIT_CHECK(!_impl->Translations.empty(), "No translations!");
			return _impl->Translations.begin()->second;
		}
		TOOLKIT_THROW("No such translation!");
	}


	std::string TranslatedString::SelectTranslation(LangCode l0) const
	{ return DoSelectTranslation(VectorBuilder<LangCode>(l0, LangCode::Any)); }


	std::string TranslatedString::SelectTranslation(LangCode l0, LangCode l1) const
	{ return DoSelectTranslation(VectorBuilder<LangCode>(l0, l1, LangCode::Any)); }


	std::string TranslatedString::SelectTranslation(LangCode l0, LangCode l1, LangCode l2) const
	{ return DoSelectTranslation(VectorBuilder<LangCode>(l0, l1, l2, LangCode::Any)); }


	std::string TranslatedString::DoSelectTranslation(const std::vector<LangCode>& langCodes) const
	{
		for (std::vector<LangCode>::const_iterator it = langCodes.begin(); it != langCodes.end(); ++it)
			if (HasTranslation(*it))
				return GetTranslation(*it);
		return "";
	}

	int TranslatedString::DoCompare(const TranslatedString& other) const
	{ return CompareMembersCmp(&TranslatedString::Impl::Translations)(_impl, other._impl);  }

	void TranslatedString::Serialize(ObjectOStream & ar) const
	{ ar.Serialize("translations", _impl->Translations); }


	void TranslatedString::Deserialize(ObjectIStream & ar)
	{ ar.Deserialize("translations", _impl->Translations); }


	std::string TranslatedString::ToString() const
	{ return stingray::ToString(_impl->Translations); }


}


