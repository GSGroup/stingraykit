#ifndef STINGRAYKIT_METAPROGRAMMING_TYPETRAITS_H
#define STINGRAYKIT_METAPROGRAMMING_TYPETRAITS_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/metaprogramming/ArrayTraits.h>
#include <stingraykit/metaprogramming/CVTraits.h>
#include <stingraykit/metaprogramming/PointerTraits.h>
#include <stingraykit/metaprogramming/ReferenceTraits.h>
#include <stingraykit/metaprogramming/If.h>
#include <stingraykit/metaprogramming/NestedTypeCheck.h>
#include <stingraykit/metaprogramming/TypeCompleteness.h>
#include <stingraykit/metaprogramming/TypeList.h>
#include <stingraykit/Types.h>

namespace stingray
{

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

	STINGRAYKIT_DECLARE_NESTED_TYPE_CHECK(Enum);
	template < typename T > struct IsEnumClass										: HasNestedType_Enum<T> { };


	typedef TypeList<u8, s8, u16, s16, u32, s32, u64, s64>																			FixedWidthIntTypes;
	typedef TypeListMerge<TypeList<FixedWidthIntTypes, TypeList<bool, char, long, unsigned long, size_t, off_t> > >::ValueT			IntTypes;
	typedef TypeList<float, double, long double>																					FloatTypes;
	typedef TypeListMerge<TypeList<IntTypes, FloatTypes> >::ValueT																	BuiltinTypes;

	template < typename T > struct IsInt											: TypeListContains<IntTypes, T> { };
	template < typename T > struct IsFixedWidthInt									: TypeListContains<FixedWidthIntTypes, T> { };
	template < typename T > struct IsFloat											: TypeListContains<FloatTypes, T> { };
	template < typename T > struct IsBuiltin										: TypeListContains<BuiltinTypes, T> { };

	namespace Detail
	{

		template < typename T, bool = IsInt<T>::Value > struct IsSignedImpl			: integral_constant<bool, T(-1) < T(0)> { };
		template < typename T > struct IsSignedImpl<T, false>						: FalseType { };

		template < typename T, bool = IsInt<T>::Value > struct IsUnsignedImpl		: integral_constant<bool, T(0) < T(-1)> { };
		template < typename T > struct IsUnsignedImpl<T, false>						: FalseType { };

	}

	template < typename T > struct IsSigned											: Detail::IsSignedImpl<T> { };
	template < typename T > struct IsUnsigned										: Detail::IsUnsignedImpl<T> { };

}

#endif
