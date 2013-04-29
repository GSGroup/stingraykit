#include <stingray/toolkit/string_stream.h>
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

	template<>
	template<>
	void basic_string_ostream<char>::Insert(unsigned value)
	{
		char buf[32];
		int r = snprintf(buf, sizeof(buf), "%u", value);
		if (r == -1)
			TOOLKIT_THROW("snprintf failed");
		_buf.insert(_buf.end(), buf, buf + r);
	}

	template<>
	template<>
	void basic_string_ostream<char>::Insert(u8 value)
	{
		char buf[32];
		int r = snprintf(buf, sizeof(buf), "%hu", (unsigned short)value);
		if (r == -1)
			TOOLKIT_THROW("snprintf failed");
		_buf.insert(_buf.end(), buf, buf + r);
	}

	template<>
	template<>
	void basic_string_ostream<char>::Insert(int value)
	{
		char buf[32];
		int r = snprintf(buf, sizeof(buf), "%d", value);
		if (r == -1)
			TOOLKIT_THROW("snprintf failed");
		_buf.insert(_buf.end(), buf, buf + r);
	}

	template<>
	template<>
	void basic_string_ostream<char>::Insert(unsigned short value)
	{
		char buf[32];
		int r = snprintf(buf, sizeof(buf), "%hu", value);
		if (r == -1)
			TOOLKIT_THROW("snprintf failed");
		_buf.insert(_buf.end(), buf, buf + r);
	}

	template<>
	template<>
	void basic_string_ostream<char>::Insert(short value)
	{
		char buf[32];
		int r = snprintf(buf, sizeof(buf), "%hd", value);
		if (r == -1)
			TOOLKIT_THROW("snprintf failed");
		_buf.insert(_buf.end(), buf, buf + r);
	}

	template<>
	template<>
	void basic_string_ostream<char>::Insert(unsigned long value)
	{
		char buf[32];
		int r = snprintf(buf, sizeof(buf), "%lu", value);
		if (r == -1)
			TOOLKIT_THROW("snprintf failed");
		_buf.insert(_buf.end(), buf, buf + r);
	}

	template<>
	template<>
	void basic_string_ostream<char>::Insert(long value)
	{
		char buf[32];
		int r = snprintf(buf, sizeof(buf), "%ld", value);
		if (r == -1)
			TOOLKIT_THROW("snprintf failed");
		_buf.insert(_buf.end(), buf, buf + r);
	}

	template<>
	template<>
	void basic_string_ostream<char>::Insert(unsigned long long value)
	{
		char buf[32];
		int r = snprintf(buf, sizeof(buf), "%llu", value);
		if (r == -1)
			TOOLKIT_THROW("snprintf failed");
		_buf.insert(_buf.end(), buf, buf + r);
	}

	template<>
	template<>
	void basic_string_ostream<char>::Insert(long long value)
	{
		char buf[32];
		int r = snprintf(buf, sizeof(buf), "%lld", value);
		if (r == -1)
			TOOLKIT_THROW("snprintf failed");
		_buf.insert(_buf.end(), buf, buf + r);
	}

	template<>
	template<>
	void basic_string_ostream<char>::Insert(long double value)
	{
		char buf[32];
		int r = snprintf(buf, sizeof(buf), "%Lg", value);
		if (r == -1)
			TOOLKIT_THROW("snprintf failed");
		_buf.insert(_buf.end(), buf, buf + r);
	}

	template<>
	template<>
	void basic_string_ostream<char>::Insert(double value)
	{
		char buf[32];
		int r = snprintf(buf, sizeof(buf), "%g", value);
		if (r == -1)
			TOOLKIT_THROW("snprintf failed");
		_buf.insert(_buf.end(), buf, buf + r);
	}

	template<>
	template<>
	void basic_string_ostream<char>::Insert(float value)
	{
		char buf[32];
		int r = snprintf(buf, sizeof(buf), "%g", (double)value);
		if (r == -1)
			TOOLKIT_THROW("snprintf failed");
		_buf.insert(_buf.end(), buf, buf + r);
	}

	template<>
	template<>
	void basic_string_ostream<char>::Insert(const void * value)
	{
		char buf[32];
		int r = snprintf(buf, sizeof(buf), "%p", value);
		if (r == -1)
			TOOLKIT_THROW("snprintf failed");
		_buf.insert(_buf.end(), buf, buf + r);
	}

	extern template void string_ostream::Insert(bool);
	extern template void string_ostream::Insert(unsigned char);
	extern template void string_ostream::Insert(short);
	extern template void string_ostream::Insert(unsigned short);
	extern template void string_ostream::Insert(long);
	extern template void string_ostream::Insert(unsigned long);
	extern template void string_ostream::Insert(long long);
	extern template void string_ostream::Insert(unsigned long long);
	extern template void string_ostream::Insert(float);
	extern template void string_ostream::Insert(double);
	extern template void string_ostream::Insert(long double);
	extern template void string_ostream::Insert(const void *);

}
