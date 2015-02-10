#ifndef STINGRAYKIT_METAPROGRAMMING_TYPETRANSFORMATIONS_H
#define STINGRAYKIT_METAPROGRAMMING_TYPETRANSFORMATIONS_H

#include <stingraykit/metaprogramming/If.h>
#include <stingraykit/metaprogramming/TypeTraits.h>

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

	template<typename T>
	struct Deconst { typedef T ValueT; };

	template<typename T>
	struct Deconst<const T> { typedef T ValueT; };

}

#endif
