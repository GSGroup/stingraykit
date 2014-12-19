#ifndef STINGRAY_TOOLKIT_LOCALE_STRINGCODEC_H
#define STINGRAY_TOOLKIT_LOCALE_STRINGCODEC_H


#include <string>
#include <stingray/toolkit/locale/Encoding.h>
#include <stingray/toolkit/locale/LocaleString.h>


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

		typedef u32 (*UnpackFunc)(std::string::const_iterator &i, const std::string::const_iterator &end);
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
