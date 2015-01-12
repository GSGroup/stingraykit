#ifndef STINGRAY_TOOLKIT_CHARARRAYPROXY_H
#define STINGRAY_TOOLKIT_CHARARRAYPROXY_H


#include <string.h>

#include <string>

#include <stingray/toolkit/MetaProgramming.h>
#include <stingray/toolkit/Types.h>
#include <stingray/toolkit/collection/array.h>
#include <stingray/toolkit/exception.h>


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
			CompileTimeAssert<sizeof(CharType) == 1>	ERROR__bigger_char_sizes_not_implemented;
			(void)ERROR__bigger_char_sizes_not_implemented;
		}

		template<typename OtherCharType, size_t OtherSize>
		CharArrayProxy& operator=(const CharArrayProxy<OtherCharType, OtherSize>& other)
		{
			CompileTimeAssert<sizeof(CharType) == sizeof(OtherCharType)> ERR_char_sizes_differ;
			CompileTimeAssert<Size >= OtherSize> ERR_array_sizes_differ;
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
