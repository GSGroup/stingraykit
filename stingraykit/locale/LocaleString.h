#ifndef STINGRAYKIT_LOCALE_LOCALESTRING_H
#define STINGRAYKIT_LOCALE_LOCALESTRING_H

// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <string>

#include <stingraykit/locale/Encoding.h>
#include <stingraykit/serialization/ISerializable.h>

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

