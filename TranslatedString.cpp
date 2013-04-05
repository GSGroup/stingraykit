#include <stingray/toolkit/TranslatedString.h>

#include <map>

#include <stingray/settings/Serialization.h>

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


	void TranslatedString::Serialize(ObjectOStream & ar) const
	{ ar.Serialize("translations", _impl->Translations); }


	void TranslatedString::Deserialize(ObjectIStream & ar)
	{ ar.Deserialize("translations", _impl->Translations); }


	std::string TranslatedString::ToString() const
	{ return stingray::ToString(_impl->Translations); }


}


