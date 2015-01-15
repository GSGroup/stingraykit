#ifndef STINGRAYKIT_STRING_UNICODE_H
#define STINGRAYKIT_STRING_UNICODE_H


#include <stingraykit/toolkit.h>
#include <stingraykit/shared_ptr.h>

#ifdef HAVE_ICU_I18N
#	include <unicode/coll.h>
#endif


namespace stingray
{

	class UnicodeCollator
	{
		STINGRAYKIT_NONCOPYABLE(UnicodeCollator);

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
	STINGRAYKIT_DECLARE_PTR(UnicodeCollator);


	std::string Utf8ToLower(const std::string& str);

}


#endif
