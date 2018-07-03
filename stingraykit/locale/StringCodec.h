#ifndef STINGRAYKIT_LOCALE_STRINGCODEC_H
#define STINGRAYKIT_LOCALE_STRINGCODEC_H

// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <string>
#include <stingraykit/locale/LocaleString.h>
#include <stingraykit/string/string_view.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_locale
	 * @{
	 */

	struct LocaleString;
	struct StringCodec
	{
		static std::string ToUtf8(const LocaleString &src, u32 invalid_char_replacement = '?');
		static LocaleString FromUtf8(const std::string &);
		static int Compare(const LocaleString &a, const LocaleString &b);

		static std::string ToCodePage(const LocaleString &src, unsigned code_page);
		static LocaleString FromCodePage(const std::string &src, unsigned code_page, u32 invalid_char_replacement = '?');

		typedef u32 (*UnpackFunc)(string_view::const_iterator &i, const string_view::const_iterator &end);
		typedef void (*PackFunc)(std::string &str, u32 unicode, char invalid_char_replacement);
		///example for(it = str.begin(); it != str.end(); ) { u32 ucs_char = (*unpack)(it, str.end()); }
		///\return pointer to unpack function, null for unsupported encoding.
		static UnpackFunc GetUnpackFunc(const Encoding &encoding);
		static void PackUtf8(std::string &dst, u32 ucs, u32 replacement);

		static PackFunc GetCodePagePackFunc(unsigned code_page);
		static UnpackFunc GetCodePageUnpackFunc(unsigned code_page);
	};

	/** @} */

}


#endif
