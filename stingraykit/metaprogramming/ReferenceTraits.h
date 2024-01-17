#ifndef STINGRAYKIT_METAPROGRAMMING_REFERENCETRAITS_H
#define STINGRAYKIT_METAPROGRAMMING_REFERENCETRAITS_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/metaprogramming/IntegralConstant.h>

namespace stingray
{

	template < typename T > struct IsLvalueReference								: FalseType	{ };
	template < typename T > struct IsLvalueReference<T&>							: TrueType	{ };

	template < typename T > struct IsRvalueReference								: FalseType	{ };
	template < typename T > struct IsRvalueReference<T&&>							: TrueType	{ };

	template < typename T > struct IsReference										: integral_constant<bool, IsLvalueReference<T>::Value || IsRvalueReference<T>::Value> { };

	template < typename T > struct RemoveReference									{ typedef T ValueT; };
	template < typename T > struct RemoveReference<T&>								{ typedef T ValueT; };
	template < typename T > struct RemoveReference<T&&>								{ typedef T ValueT; };

	template < typename T > struct AddLvalueReference								{ typedef T& ValueT; };
	template < > struct AddLvalueReference<void>									{ typedef void ValueT; };
	template < > struct AddLvalueReference<const void>								{ typedef const void ValueT; };
	template < typename T > struct AddLvalueReference<T&>							{ typedef T& ValueT; };
	template < typename T > struct AddLvalueReference<T&&>							{ typedef T& ValueT; };

	template < typename T > struct AddRvalueReference								{ typedef T&& ValueT; };
	template < > struct AddRvalueReference<void>									{ typedef void ValueT; };
	template < > struct AddRvalueReference<const void>								{ typedef const void ValueT; };
	template < typename T > struct AddRvalueReference<T&&>							{ typedef T&& ValueT; };

}

#endif
