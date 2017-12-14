#ifndef STINGRAYKIT_METAPROGRAMMING_TYPETRAITS_H
#define STINGRAYKIT_METAPROGRAMMING_TYPETRAITS_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/metaprogramming/If.h>
#include <stingraykit/metaprogramming/IntegralConstant.h>
#include <stingraykit/metaprogramming/TypeList.h>
#include <stingraykit/metaprogramming/YesNo.h>
#include <stingraykit/Types.h>

#include <stddef.h>

namespace stingray
{

// Const-volatile
	template < typename T > struct IsConst											: FalseType	{ };
	template < typename T > struct IsConst<const T>									: TrueType	{ };

	template < typename T > struct IsVolatile										: FalseType	{ };
	template < typename T > struct IsVolatile<volatile T>							: TrueType	{ };

	template < typename T > struct RemoveConst										{ typedef T ValueT; };
	template < typename T > struct RemoveConst<const T>								{ typedef T ValueT; };

	template < typename T > struct AddConst											{ typedef const T ValueT; };

	template < typename T > struct RemoveVolatile									{ typedef T ValueT; };
	template < typename T > struct RemoveVolatile<volatile T>						{ typedef T ValueT; };

	template < typename T > struct AddVolatile										{ typedef volatile T ValueT; };

	template < typename T > struct RemoveCV											{ typedef typename RemoveVolatile<typename RemoveConst<T>::ValueT>::ValueT ValueT; };
	template < typename T > struct AddCV											{ typedef const volatile T ValueT; };

// Reference
	template < typename T > struct IsReference										: FalseType	{ };
	template < typename T > struct IsReference<T&>									: TrueType	{ };

	template < typename T > struct RemoveReference									{ typedef T ValueT; };
	template < typename T > struct RemoveReference<T&>								{ typedef T ValueT; };

	template < typename T > struct AddReference										{ typedef T& ValueT; };
	template < typename T > struct AddReference<T&>									{ typedef T& ValueT; };

// Pointer
	namespace Detail
	{

		template < typename T > struct IsPointerImpl								: FalseType	{ };
		template < typename T > struct IsPointerImpl<T*>							: TrueType	{ };

	}
	template < typename T > struct IsPointer										: Detail::IsPointerImpl<typename RemoveCV<T>::ValueT> { };

	template < typename T > struct RemovePointer									{ typedef T ValueT; };
	template < typename T > struct RemovePointer<T*>								{ typedef T ValueT; };
	template < typename T > struct RemovePointer<T* const>							{ typedef T ValueT; };
	template < typename T > struct RemovePointer<T* volatile>						{ typedef T ValueT; };
	template < typename T > struct RemovePointer<T* const volatile>					{ typedef T ValueT; };

	template < typename T > struct AddPointer										{ typedef typename RemoveReference<T>::ValueT* ValueT; };

	template < typename T > struct ToPointer										{ typedef T* ValueT; };
	template < typename T > struct ToPointer<T&>									{ typedef T* ValueT; };
	template < typename T > struct ToPointer<T*>									{ typedef T* ValueT; };
	template < typename T > struct ToPointer<T* const>								{ typedef T* ValueT; };
	template < typename T > struct ToPointer<T* volatile>							{ typedef T* ValueT; };
	template < typename T > struct ToPointer<T* const volatile>						{ typedef T* ValueT; };

// Array
	template < typename T > struct IsArray											: FalseType	{ };
	template < typename T > struct IsArray<T[]>										: TrueType	{ };
	template < typename T, size_t N > struct IsArray<T[N]>							: TrueType	{ };

	template < typename T > struct RemoveExtent										{ typedef T ValueT; };
	template < typename T > struct RemoveExtent<T[]>								{ typedef T ValueT; };
	template < typename T, size_t N> struct RemoveExtent<T[N]>						{ typedef T ValueT; };

// Miscellaneous
	template < typename T > struct IsConstReference									: integral_constant<bool, IsReference<T>::Value && IsConst<typename RemoveReference<T>::ValueT>::Value> { };
	template < typename T > struct IsNonConstReference								: integral_constant<bool, IsReference<T>::Value && !IsConstReference<T>::Value> { };

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

#if defined(__GNUC__) || defined(__clang__)
	template < typename T > struct IsUnion											: integral_constant<bool, __is_union(typename RemoveCV<T>::ValueT)> { };
#else
	template < typename T > struct IsUnion											: FalseType { };
#endif

	namespace Detail
	{

		template < typename T > YesType	TestIsClass(int T::*);
		template < typename T > NoType	TestIsClass(...);

	}
	template < typename T > struct IsClass											: integral_constant<bool, ( sizeof(Detail::TestIsClass<T>(0)) == sizeof(YesType) ) && !IsUnion<T>::Value> { };

	template < typename T > struct IsComplete										: integral_constant<bool, sizeof(T) == sizeof(T)> { };


	typedef TypeList<u8, s8, u16, s16, u32, s32, u64, s64>::type																	FixedWidthIntTypes;
	typedef TypeListMerge<TypeList_2<FixedWidthIntTypes, TypeList<unsigned, unsigned long, size_t, int, long, off_t> > >::ValueT	IntTypes;
	typedef TypeListMerge<TypeList_2<IntTypes, TypeList<float, double, bool> > >::ValueT											BuiltinTypes;

	template < typename T > struct IsInt											: TypeListContains<IntTypes, T> { };
	template < typename T > struct IsFixedWidthInt									: TypeListContains<FixedWidthIntTypes, T> { };
	template < typename T > struct IsBuiltin										: TypeListContains<BuiltinTypes, T> { };

	namespace Detail
	{

		template < typename T, bool = IsInt<T>::Value > struct IsSignedImpl			: integral_constant<bool, ~T(0) < T(0)> { };
		template < typename T > struct IsSignedImpl<T, false>						: FalseType { };

		template < typename T, bool = IsInt<T>::Value > struct IsUnsignedImpl		: integral_constant<bool, T(0) <~ T(0)> { };
		template < typename T > struct IsUnsignedImpl<T, false>						: FalseType { };

	}

	template < typename T > struct IsSigned											: Detail::IsSignedImpl<T> { };
	template < typename T > struct IsUnsigned										: Detail::IsUnsignedImpl<T> { };

}

#endif
