#ifndef STINGRAYKIT_METAPROGRAMMING_TYPETRAITS_H
#define STINGRAYKIT_METAPROGRAMMING_TYPETRAITS_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/metaprogramming/CompileTimeAssert.h>
#include <stingraykit/metaprogramming/If.h>
#include <stingraykit/metaprogramming/ToVoid.h>

#include <stddef.h>

namespace stingray
{

// Const-volatile
	template < typename T > struct IsConst											{ static const bool Value = false; };
	template < typename T > struct IsConst<const T>									{ static const bool Value = true; };

	template < typename T > struct IsVolatile										{ static const bool value = false; };
	template < typename T > struct IsVolatile<volatile T>							{ static const bool value = true; };

	template < typename T > struct RemoveConst										{ typedef T ValueT; };
	template < typename T > struct RemoveConst<const T>								{ typedef T ValueT; };

	template < typename T > struct AddConst											{ typedef const T ValueT; };

	template < typename T > struct RemoveVolatile									{ typedef T ValueT; };
	template < typename T > struct RemoveVolatile<volatile T>						{ typedef T ValueT; };

	template < typename T > struct AddVolatile										{ typedef volatile T ValueT; };

	template < typename T > struct RemoveCV											{ typedef typename RemoveVolatile<typename RemoveConst<T>::ValueT>::ValueT ValueT; };
	template < typename T > struct AddCV											{ typedef const volatile T ValueT; };

// Reference
	template < typename T > struct IsReference										{ static const bool Value = false; };
	template < typename T > struct IsReference<T&>									{ static const bool Value = true; };

	template < typename T > struct RemoveReference									{ typedef T ValueT; };
	template < typename T > struct RemoveReference<T&>								{ typedef T ValueT; };

	template < typename T > struct AddReference										{ typedef T& ValueT; };
	template < typename T > struct AddReference<T&>									{ typedef T& ValueT; };

// Pointer
	namespace Detail
	{

		template < typename T > struct IsPointerImpl								{ static const bool Value = false; };
		template < typename T > struct IsPointerImpl<T*>							{ static const bool Value = true; };

	}
	template < typename T > struct IsPointer : Detail::IsPointerImpl<typename RemoveCV<T>::ValueT> { };

	template < typename T > struct RemovePointer									{ typedef T ValueT; };
	template < typename T > struct RemovePointer<T*>								{ typedef T ValueT; };
	template < typename T > struct RemovePointer<T* const>							{ typedef T ValueT; };
	template < typename T > struct RemovePointer<T* volatile>						{ typedef T ValueT; };
	template < typename T > struct RemovePointer<T* const volatile>					{ typedef T ValueT; };

	template < typename T > struct AddPointer										{ typedef typename RemoveReference<T>::ValueT* ValueT; };

// Array
	template < typename T > struct IsArray											{ static const bool Value = false; };
	template < typename T, size_t N > struct IsArray<T[N]>							{ static const bool Value = true; };

	template < typename T > struct RemoveExtent										{ typedef T ValueT; };
	template < typename T > struct RemoveExtent<T[]>								{ typedef T ValueT; };
	template < typename T, size_t N> struct RemoveExtent<T[N]>						{ typedef T ValueT; };

// Miscellaneous
	template < typename T > struct IsConstReference									{ static const bool Value = IsReference<T>::Value && IsConst<typename RemoveReference<T>::ValueT>::Value; };
	template < typename T > struct IsNonConstReference								{ static const bool Value = IsReference<T>::Value && !IsConstReference<T>::Value; };

	template < typename T > struct AddConstReference								{ typedef typename AddReference<typename AddConst<T>::ValueT>::ValueT ValueT; };
	template < typename T > struct AddConstPointer									{ typedef typename AddPointer<typename AddConst<T>::ValueT>::ValueT ValueT; };

	template < typename T > struct RemoveCVReference								{ typedef typename RemoveCV<typename RemoveReference<T>::ValueT>::ValueT ValueT; };

	template < typename T > struct Decay
	{
	private:
		typedef typename RemoveReference<T>::ValueT _T;

	public:
		typedef typename If<IsArray<_T>::Value, typename RemoveExtent<_T>::ValueT*, typename RemoveCV<_T>::ValueT>::ValueT ValueT;
	};

	template < typename T, typename MemberPointerDetector = void > struct IsClass	{ static const bool Value = false; };
	template < typename T > struct IsClass<T, typename ToVoid<int T::*>::ValueT>	{ static const bool Value = true; };

	template < typename T > struct StaticAssertCompleteType : CompileTimeAssert<sizeof(T) == sizeof(T)> { };

}

#endif
