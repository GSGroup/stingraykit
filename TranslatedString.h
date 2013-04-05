#ifndef STINGRAY_SCANNER_TRANSLATEDSTRING_H
#define STINGRAY_SCANNER_TRANSLATEDSTRING_H

#include <stingray/locale/LangCode.h>
#include <stingray/locale/LocaleString.h>
#include <stingray/settings/ISerializable.h>


namespace stingray
{


	struct TranslatedString
	{
	private:
		struct Impl;
		shared_ptr<Impl>	_impl;

	public:
		TranslatedString();
		TranslatedString(const TranslatedString& other);
		~TranslatedString();

		TranslatedString& operator =(const TranslatedString& other);

		void AddTranslation(LangCode lang, const LocaleString& str);

		bool HasTranslation(LangCode lang) const;
		LocaleString GetTranslation(LangCode lang) const;

		void Serialize(ObjectOStream &ar) const;
		void Deserialize(ObjectIStream &ar);

		std::string ToString() const;
	};


}


#endif

