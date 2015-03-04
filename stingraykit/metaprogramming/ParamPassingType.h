#ifndef STINGRAYKIT_METAPROGRAMMING_PARAMPASSINGTYPE_H
#define STINGRAYKIT_METAPROGRAMMING_PARAMPASSINGTYPE_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

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
