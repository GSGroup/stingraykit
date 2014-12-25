#ifndef STINGRAY_TOOLKIT_LOCALE_LOCALESTRING_H
#define STINGRAY_TOOLKIT_LOCALE_LOCALESTRING_H

#include <string>

#include <stingray/toolkit/locale/Encoding.h>
#include <stingray/toolkit/serialization/ISerializable.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_locale
	 * @{
	 */

	struct LocaleString
	{
		Encoding		TextEncoding;
		std::string		Text;

		LocaleString();
		LocaleString(Encoding encoding, const std::string& text);
		LocaleString(const LocaleString& l);
		~LocaleString();

		void Serialize(ObjectOStream & ar) const;
		void Deserialize(ObjectIStream & ar);

		bool operator<(const LocaleString& other) const;
		bool operator==(const LocaleString& other) const;
		bool operator!=(const LocaleString& other) const;

		std::string ToString() const;
		static LocaleString FromUtf8(const std::string &str);

		int Compare(const LocaleString &other) const;
	};

	template<typename T>
	std::basic_ostream<T>& operator << (std::basic_ostream<T>& stream, const LocaleString& val)
	{
		return stream << val.ToString();
	}

	/** @} */

}

#endif	/* LOCALE_LOCALESTRING_H */

