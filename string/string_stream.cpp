#include <stingray/toolkit/string/string_stream.h>
#include <stingray/toolkit/exception.h>
#include <stdio.h>

namespace stingray
{
	template<>
	template<>
	void basic_string_ostream<char>::Insert(bool value)
	{
		if (value)
			write("true", 4);
		else
			write("false", 5);
	}
	extern template void string_ostream::Insert(bool);

#define DECLARE_INSERT(VALUE_TYPE, VALUE_FORMAT, VALUE_FORMAT_TYPE) \
	template<> \
	template<> \
	void basic_string_ostream<char>::Insert(VALUE_TYPE value) \
	{ \
		char buf[32]; \
		int r = snprintf(buf, sizeof(buf), VALUE_FORMAT, static_cast<VALUE_FORMAT_TYPE>(value)); \
		if (r == -1) \
			TOOLKIT_THROW("snprintf failed"); \
		write(buf, r); \
	} \
	extern template void string_ostream::Insert(VALUE_TYPE)

	DECLARE_INSERT(unsigned,			"%u",	unsigned);
	DECLARE_INSERT(u8,					"%hu",	unsigned short);
	DECLARE_INSERT(int,					"%d",	int);
	DECLARE_INSERT(unsigned short,		"%hu",	unsigned short);
	DECLARE_INSERT(short,				"%hd",	short);
	DECLARE_INSERT(unsigned long,		"%lu",	unsigned long);
	DECLARE_INSERT(long,				"%ld",	long);
	DECLARE_INSERT(unsigned long long,	"%llu",	unsigned long long);
	DECLARE_INSERT(long long,			"%lld",	long long);
	DECLARE_INSERT(long double,			"%16.16Lg",long double);
	DECLARE_INSERT(double,				"%16.16g",	double);
	DECLARE_INSERT(float,				"%7.7g",	double);
	DECLARE_INSERT(const void *,		"%p",	const void *);

}
