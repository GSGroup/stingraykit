#ifndef STINGRAYKIT_STRING_TOSTRINGFORWARD_H
#define STINGRAYKIT_STRING_TOSTRINGFORWARD_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/string/string_stream.h>

namespace stingray
{

	namespace Detail
	{
		template < typename StringType >
		struct TypeFromStringInterpreter;

		template < typename StringType >
		struct TypeTryFromStringInterpreter;

		template < typename T >
		struct TypeToStringSerializer;
	}


	template < typename T, typename StringType >
	auto FromString(const StringType& str)
			-> typename RemoveReference<decltype(Detail::TypeFromStringInterpreter<StringType>::template FromStringImpl<T>(str, 0))>::ValueT;


	template < typename T >
	struct FromStringInterpreter;


	template < typename T, typename StringType >
	auto TryFromString(const StringType& str)
		-> typename RemoveReference<decltype(Detail::TypeTryFromStringInterpreter<StringType>::template TryFromStringImpl<T>(str, 0))>::ValueT;


	template < typename T >
	struct TryFromStringInterpreter;


	template < typename T >
	auto ToString(string_ostream& result, const T& val)
			-> decltype(Detail::TypeToStringSerializer<T>::ToStringImpl(result, val, 0), void());


	template < typename T >
	auto ToString(const T& val)
			-> decltype(ToString(std::declval<string_ostream&>(), val), std::string());


	struct ToStringPrinter
	{
		using RetType = std::string;

		template < typename T >
		std::string operator () (const T& val) const;
	};

}

#endif
