#ifndef STINGRAYKIT_METAPROGRAMMING_PARAMPASSINGTYPE_H
#define STINGRAYKIT_METAPROGRAMMING_PARAMPASSINGTYPE_H

#include <stingraykit/metaprogramming/EnableIf.h>
#include <stingraykit/metaprogramming/NestedTypeCheck.h>
#include <stingraykit/metaprogramming/TypeTraits.h>

namespace stingray
{

	namespace Detail_ParamPassingType
	{
		STINGRAYKIT_DECLARE_NESTED_TYPE_CHECK(Enum);

		template < typename T, typename Enabler = void >
		struct PassByRef
		{ static const bool Value = true; };

		template < typename T >
		struct PassByRef<T, typename EnableIf<IsBuiltinType<T>::Value, void>::ValueT>
		{ static const bool Value = false; };

		template < typename T >
		struct PassByRef<T, typename EnableIf<IsPointer<T>::Value, void>::ValueT>
		{ static const bool Value = false; };

		template < typename T >
		struct PassByRef<T, typename EnableIf<HasNestedType_Enum<T>::Value, void>::ValueT>
		{ static const bool Value = false; };
	}


	template < typename T >
	struct GetParamPassingType
	{
		typedef typename Dereference<typename Deconst<T>::ValueT>::ValueT RawType;
		typedef typename If<Detail_ParamPassingType::PassByRef<T>::Value, const RawType&, RawType>::ValueT ConstPassingType;
		typedef typename If<IsNonConstReference<T>::Value, RawType&, ConstPassingType>::ValueT ValueT;
	};

}

#endif
