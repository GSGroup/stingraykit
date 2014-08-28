#include <stingray/toolkit/Unicode.h>
#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/StringUtils.h>


namespace stingray
{

#ifdef HAVE_ICU_I18N

	UnicodeCollator::UnicodeCollator() : _collator()
	{
		UErrorCode success = U_ZERO_ERROR;
		_collator = icu::Collator::createInstance(success);
		TOOLKIT_CHECK(success == U_ZERO_ERROR, "creating collator failed");
	}

	void UnicodeCollator::SetCaseSensitivity(bool sensitive)
	{
		_collator->setStrength(sensitive? icu::Collator::TERTIARY: icu::Collator::PRIMARY);
	}

	int UnicodeCollator::Compare(const std::string &str1, const std::string &str2) const
	{
		CompileTimeAssert<(UCOL_EQUAL == 0)>	Error_invalid_equal_const;
		CompileTimeAssert<(UCOL_LESS < 0)>		Error_invalid_less_const;
		CompileTimeAssert<(UCOL_GREATER > 0)>	Error_invalid_greater_const;

		UErrorCode success = U_ZERO_ERROR;
		UCollationResult r = _collator->compareUTF8(str1, str2, success);
		TOOLKIT_CHECK(success == U_ZERO_ERROR, "compareUTF8 failed");
		return r;
	}

	UnicodeCollator::~UnicodeCollator()
	{
		delete _collator;
	}

#else
	UnicodeCollator::UnicodeCollator() { }

	void UnicodeCollator::SetCaseSensitivity(bool sensitive)
	{ _caseSensitive = sensitive; }

	int UnicodeCollator::Compare(const std::string &str1, const std::string &str2) const
	{
		return _caseSensitive? str1.compare(str2): Utf8ToLower(str1).compare(Utf8ToLower(str2));
	}

	UnicodeCollator::~UnicodeCollator()
	{ }

#endif

}
