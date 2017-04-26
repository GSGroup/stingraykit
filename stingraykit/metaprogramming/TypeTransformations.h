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

	template < typename T >
	struct GetConstReferenceType
	{ typedef const T&	ValueT; };

	template < typename T >
	struct GetConstReferenceType<T&>
	{ typedef const T&	ValueT; };

	template < typename T >
	struct GetConstReferenceType<const T&>
	{ typedef const T&	ValueT; };

	template < typename T >
	struct GetConstPointerType
	{ typedef const T*	ValueT; };

	template < typename T >
	struct Dereference { typedef T	ValueT; };

	template < typename T >
	struct Dereference<T&> { typedef T	ValueT; };

	template < typename T >
	struct Depointer { typedef T	ValueT; };

	template < typename T >
	struct Depointer<T*> { typedef T	ValueT; };

	template < typename T >
	struct Depointer<T* const> { typedef T	ValueT; };

	template<typename T>
	struct Deconst { typedef T ValueT; };

	template<typename T>
	struct Deconst<const T> { typedef T ValueT; };


	template <typename T>
	struct RemoveExtent
	{ typedef T ValueT; };


	template <typename T>
	struct RemoveExtent<T[]>
	{ typedef T ValueT; };


	template <typename T, size_t N>
	struct RemoveExtent<T[N]>
	{ typedef T ValueT; };

}

#endif
