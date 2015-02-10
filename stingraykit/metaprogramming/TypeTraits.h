#ifndef STINGRAYKIT_METAPROGRAMMING_TYPETRAITS_H
#define STINGRAYKIT_METAPROGRAMMING_TYPETRAITS_H

#include <stingraykit/metaprogramming/ToVoid.h>

namespace stingray
{

	template < typename T >
	struct IsReference { static const bool Value = false; };

	template < typename T >
	struct IsReference<T&> { static const bool Value = true; };

	template < typename T >
	struct IsConstReference { static const bool Value = false; };

	template < typename T >
	struct IsConstReference<const T&> { static const bool Value = true; };

	template < typename T >
	struct IsNonConstReference { static const bool Value = IsReference<T>::Value && !IsConstReference<T>::Value; };

	template < typename T >
	struct IsConst { static const bool Value = false; };

	template < typename T >
	struct IsConst<const T> { static const bool Value = true; };

	template < typename T >
	struct IsPointer { static const bool Value = false; };

	template < typename T >
	struct IsPointer<T*> { static const bool Value = true; };

	template < typename T >
	struct IsArray { static const bool Value = false; };

	template < typename T, unsigned N >
	struct IsArray<T[N]> { static const bool Value = true; };

	template < typename T, typename MemberPointerDetector = void >
	struct IsClass
	{ static const bool Value = false; };

	template < typename T >
	struct IsClass<T, typename ToVoid<int T::*>::ValueT >
	{ static const bool Value = true; };


	template < typename T >
	struct GetPointedType;

	template < typename T >
	struct GetPointedType<T *>
	{ typedef T ValueT; };

}

#endif
