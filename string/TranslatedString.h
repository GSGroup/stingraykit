#ifndef STINGRAYKIT_STRING_TRANSLATEDSTRING_H
#define STINGRAYKIT_STRING_TRANSLATEDSTRING_H

#include <stingray/toolkit/locale/LangCode.h>
#include <stingray/toolkit/serialization/ISerializable.h>


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

		void AddTranslation(LangCode lang, const std::string& str);

		bool HasTranslation(LangCode lang) const;
		std::string GetTranslation(LangCode lang) const;

		void Serialize(ObjectOStream &ar) const;
		void Deserialize(ObjectIStream &ar);

		std::string ToString() const;

		std::string SelectTranslation(LangCode l0) const;
		std::string SelectTranslation(LangCode l0, LangCode l1) const;
		std::string SelectTranslation(LangCode l0, LangCode l1, LangCode l2) const;

		bool operator < (const TranslatedString& other) const
		{ return DoCompare(other) < 0; }

	private:
		std::string DoSelectTranslation(const std::vector<LangCode>& langCodes) const;
		int DoCompare(const TranslatedString& other) const;
	};


}


#endif

