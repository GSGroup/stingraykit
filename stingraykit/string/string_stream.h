#ifndef STINGRAYKIT_STRING_STRING_STREAM_H
#define STINGRAYKIT_STRING_STRING_STREAM_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#define STRING_STREAM_USES_INPLACE_VECTOR (PLATFORM_LINUX)

#if STRING_STREAM_USES_INPLACE_VECTOR
#	include <stingraykit/collection/inplace_vector.h>
#else
#	include <vector>
#endif

#include <string>

namespace stingray
{

	template < typename CharType >
	class basic_string_ostream
	{
	public:
		using value_type = CharType;
		using const_reference = const value_type;
		using const_pointer = const value_type*;
		using reference = value_type;
		using pointer = value_type*;

	private:
		static const unsigned InplaceCapacity = 256;

#if STRING_STREAM_USES_INPLACE_VECTOR
		inplace_vector<value_type, InplaceCapacity>	_buf;
#else
		std::vector<value_type>						_buf;
#endif

		void reserve(size_t size)
		{
			//reserving data in InplaceCapacity bytes chunks
			const size_t capacity = _buf.capacity();
			if (_buf.size() + size > capacity)
				_buf.reserve(capacity + (size + InplaceCapacity - 1) / InplaceCapacity * InplaceCapacity);
		}

	public:
		bool empty() const { return _buf.empty(); }

		std::string str() const
		{ return std::string(_buf.begin(), _buf.end()); }

		void str(const std::string& value)
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

		basic_string_ostream& operator << (const std::string& value)
		{ Insert(value); return *this; }

		basic_string_ostream& operator << (float value)
		{ Insert(value); return *this; }

		basic_string_ostream& operator << (double value)
		{ Insert(value); return *this; }

		basic_string_ostream& operator << (long double value)
		{ Insert(value); return *this; }

		basic_string_ostream& operator << (const char* value)
		{ Insert(value); return *this; }

		basic_string_ostream& operator << (const void* value)
		{ Insert(value); return *this; }

		void write(const_pointer data, size_t size)
		{
			reserve(size);
			std::copy(data, data + size, std::back_inserter(_buf));
		}

		void push_back(value_type c) { Insert(c); }

	private:
		void Insert(value_type value)
		{ _buf.push_back(value); }

		void Insert(const char* value)
		{
			while (*value)
				_buf.push_back(*value++);
		}

		void Insert(const std::basic_string<value_type>& value)
		{
			reserve(value.size());
			std::copy(value.begin(), value.end(), std::back_inserter(_buf));
		}

		template < typename T >
		void Insert(T value);

		template < typename T >
		void InsertIntegral(T value);
	};

	using string_ostream = basic_string_ostream<char>;

}

#endif
