#ifndef STINGRAYKIT_METAPROGRAMMING_TYPETRANSFORMATIONS_H
#define STINGRAYKIT_METAPROGRAMMING_TYPETRANSFORMATIONS_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/metaprogramming/If.h>
#include <stingraykit/metaprogramming/TypeTraits.h>

#include <stddef.h>

namespace stingray
{

// Const-volatile
	template < typename T > struct RemoveConst						{ typedef T ValueT; };
	template < typename T > struct RemoveConst<const T>				{ typedef T ValueT; };

	template < typename T > struct AddConst							{ typedef const T ValueT; };

	template < typename T > struct RemoveVolatile					{ typedef T ValueT; };
	template < typename T > struct RemoveVolatile<volatile T>		{ typedef T ValueT; };

	template < typename T > struct AddVolatile						{ typedef volatile T ValueT; };

	template < typename T > struct RemoveCV							{ typedef typename RemoveVolatile<typename RemoveConst<T>::ValueT>::ValueT ValueT; };
	template < typename T > struct AddCV							{ typedef const volatile T ValueT; };

// Reference
	template < typename T > struct RemoveReference					{ typedef T ValueT; };
	template < typename T > struct RemoveReference<T&>				{ typedef T ValueT; };

	template < typename T > struct AddReference						{ typedef T& ValueT; };
	template < typename T > struct AddReference<T&>					{ typedef T& ValueT; };

// Pointer
	template < typename T > struct RemovePointer					{ typedef T ValueT; };
	template < typename T > struct RemovePointer<T*>				{ typedef T ValueT; };
	template < typename T > struct RemovePointer<T* const>			{ typedef T ValueT; };
	template < typename T > struct RemovePointer<T* volatile>		{ typedef T ValueT; };
	template < typename T > struct RemovePointer<T* const volatile>	{ typedef T ValueT; };

	template < typename T > struct AddPointer						{ typedef typename RemoveReference<T>::ValueT* ValueT; };

// Array
	template < typename T > struct RemoveExtent						{ typedef T ValueT; };
	template < typename T > struct RemoveExtent<T[]>				{ typedef T ValueT; };
	template < typename T, size_t N> struct RemoveExtent<T[N]>		{ typedef T ValueT; };

// Miscellaneous transformations
	template < typename T > struct AddConstReference				{ typedef const T& ValueT; };
	template < typename T > struct AddConstReference<T&>			{ typedef const T& ValueT; };

	template < typename T > struct AddConstPointer					{ typedef const T* ValueT; };

}

#endif
