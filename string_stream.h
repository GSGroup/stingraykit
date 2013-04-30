#ifndef STINGRAY_TOOLKIT__STRING_STREAM_H
#define STINGRAY_TOOLKIT__STRING_STREAM_H

#include <deque>
#include <string>

namespace stingray
{

	template<typename CharType>
	class basic_string_ostream
	{
	public:
		typedef	CharType			value_type;
		typedef const value_type	const_reference;
		typedef const value_type *	const_pointer;
		typedef value_type			reference;
		typedef value_type *		pointer;

	private:
		std::deque<value_type>		_buf;

	public:
		inline bool empty() const { return _buf.empty(); }

		std::string str() const
		{ return std::string(_buf.begin(), _buf.end()); }

		void str(const std::string &value)
		{ _buf.assign(value.begin(), value.end()); }

		basic_string_ostream& operator << (bool value)
		{ Insert(value); return *this; }

		basic_string_ostream& operator << (char value)
		{ Insert(value); return *this; }

		basic_string_ostream& operator << (unsigned char value)
		{ Insert(value); return *this; }

		basic_string_ostream& operator << (short value)
		{ Insert(value); return *this; }

		basic_string_ostream& operator << (unsigned short value)
		{ Insert(value); return *this; }

		basic_string_ostream& operator << (int value)
		{ Insert(value); return *this; }

		basic_string_ostream& operator << (unsigned int value)
		{ Insert(value); return *this; }

		basic_string_ostream& operator << (long value)
		{ Insert(value); return *this; }

		basic_string_ostream& operator << (unsigned long value)
		{ Insert(value); return *this; }

		basic_string_ostream& operator << (long long value)
		{ Insert(value); return *this; }

		basic_string_ostream& operator << (unsigned long long value)
		{ Insert(value); return *this; }

		basic_string_ostream& operator << (const std::string &value)
		{ Insert(value); return *this; }

		basic_string_ostream& operator << (float value)
		{ Insert(value); return *this; }

		basic_string_ostream& operator << (double value)
		{ Insert(value); return *this; }

		basic_string_ostream& operator << (long double value)
		{ Insert(value); return *this; }

		basic_string_ostream& operator << (const char *value)
		{ Insert(value); return *this; }

		basic_string_ostream& operator << (const void *value)
		{ Insert(value); return *this; }


		inline void write(const_pointer data, size_t size)
		{ _buf.insert(_buf.end(), data, data +size); }

		inline void push_back(value_type c) { Insert(c); }

	private:
		basic_string_ostream& operator<<(std::ios_base& (*__pf) (std::ios_base&));

		void Insert(value_type value)
		{ _buf.push_back(value); }

		void Insert(const char *value)
		{ while(*value) _buf.push_back(*value++); }

		void Insert(const std::basic_string<value_type>& value)
		{ _buf.insert(_buf.end(), value.begin(), value.end()); }

		template<typename T>
		void Insert(T value);
	};

	typedef basic_string_ostream<char> string_ostream;

}


#endif
