#ifndef STINGRAYKIT_CHARARRAYPROXY_H
#define STINGRAYKIT_CHARARRAYPROXY_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/array.h>
#include <stingraykit/exception.h>

#include <string.h>

namespace stingray
{

	template< typename CharType, size_t Size >
	struct CharArrayProxy
	{
		typedef typename If<IsConst<CharType>::Value, const char*, char*>::ValueT	PtrType;
	private:
		PtrType _array;

	public:
		CharArrayProxy(CharType* array) : _array(reinterpret_cast<PtrType>(array))
		{
			static_assert(sizeof(CharType) == 1, "Bigger char sizes not implemented");
		}

		template<typename OtherCharType, size_t OtherSize>
		CharArrayProxy& operator=(const CharArrayProxy<OtherCharType, OtherSize>& other)
		{
			static_assert(sizeof(CharType) == sizeof(OtherCharType), "Char sizes differ");
			static_assert(Size >= OtherSize, "Array sizes differ");
			strncpy(_array, other._array, Size);
			return *this;
		}

		CharArrayProxy& operator=(const std::string& str)
		{
			STINGRAYKIT_CHECK(Size > str.size(), ArgumentException("str", str));
			strncpy(_array, str.c_str(), Size);
			return *this;
		}

		CharArrayProxy& operator=(const char* str)
		{
			STINGRAYKIT_CHECK(Size > strlen(str), ArgumentException("str", str));
			strncpy(_array, str, Size);
			return *this;
		}
	};


	template< size_t Size >
	struct StringToCharArrayHelper
	{
	private:
		array<char, Size> _array;

	public:
		StringToCharArrayHelper(const std::string& str)
		{
			STINGRAYKIT_CHECK(Size > str.size(), ArgumentException("str", str));
			strncpy(_array.data(), str.c_str(), Size);
		}

		operator char*() { return _array.data(); }
	};

}

#endif
