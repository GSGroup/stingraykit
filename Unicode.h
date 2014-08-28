#ifndef STINGRAY_TOOLKIT_UNICODE_H
#define STINGRAY_TOOLKIT_UNICODE_H

#include <stingray/toolkit/toolkit.h>

#ifdef HAVE_ICU_I18N
#	include <unicode/coll.h>
#endif


namespace stingray
{

	class UnicodeCollator
	{
		TOOLKIT_NONCOPYABLE(UnicodeCollator);

	private:
#ifdef HAVE_ICU_I18N
		icu::Collator *		_collator;
#endif
		bool				_caseSensitive;

	public:
		UnicodeCollator();
		~UnicodeCollator();

		void SetCaseSensitivity(bool sensitive);
		int Compare(const std::string &str1, const std::string &str2) const;
	};

	std::string Utf8ToLower(const std::string& str);

}


#endif
