#ifndef STINGRAYKIT_STRING_STRING_STREAM_H
#define STINGRAYKIT_STRING_STRING_STREAM_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/inplace_vector.h>
#include <stingraykit/string/string_view.h>

namespace stingray
{

	template < typename CharType >
	class basic_string_ostream
	{
		STINGRAYKIT_NONCOPYABLE(basic_string_ostream);

	public:
		using value_type = CharType;
		using const_reference = const value_type;
		using const_pointer = const value_type*;
		using reference = value_type;
		using pointer = value_type*;

		using string_type = std::basic_string<value_type>;
		using string_view_type = basic_string_view<value_type>;

	private:
		static const unsigned InplaceCapacity = 256;

		inplace_vector<value_type, InplaceCapacity>		_buf;

	public:
		basic_string_ostream() { }

		bool empty() const { return _buf.empty(); }

		void clear() { _buf.clear(); }

		string_type str() const
		{ return string_type(_buf.begin(), _buf.end()); }

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

		basic_string_ostream& operator << (const string_type& value)
		{ Insert(string_view_type(value)); return *this; }

		basic_string_ostream& operator << (string_view_type value)
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

		void push_back(value_type c) { Insert(c); }

	private:
		void Reserve(size_t size)
		{
			//reserving data in InplaceCapacity bytes chunks
			const size_t capacity = _buf.capacity();
			if (_buf.size() + size > capacity)
				_buf.reserve(capacity + (size + InplaceCapacity - 1) / InplaceCapacity * InplaceCapacity);
		}

		void Insert(value_type value)
		{ _buf.push_back(value); }

		void Insert(const char* value)
		{
			if (value)
			{
				while (*value)
					_buf.push_back(*value++);
			}
			else
				Insert(static_cast<const void*>(value));
		}

		void Insert(string_view_type value)
		{
			Reserve(value.size());
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
