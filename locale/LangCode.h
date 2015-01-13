#ifndef STINGRAYKIT_LOCALE_LANGCODE_H
#define STINGRAYKIT_LOCALE_LANGCODE_H

#include <string>

#include <stingray/toolkit/toolkit.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_locale
	 * @{
	 */

	class LangCode
	{
		u32 _code;

	public:

		struct AnyType {};
		static const AnyType Any;

		inline LangCode() : _code(0) { }
		inline LangCode(AnyType) : _code(0) { }

		explicit LangCode(u32 code);
		LangCode(const std::string &code);
		LangCode(const char *code);

		inline bool operator <  (const LangCode& other) const	{ return _code < other._code; }
		STINGRAYKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(LangCode);

		static LangCode FromString(const std::string &code)		{ return LangCode(code); }
		std::string ToString() const;

	private:
		void ToUpper();
		static char DoToUpper(char c);
	};

	/** @} */

}


#endif
