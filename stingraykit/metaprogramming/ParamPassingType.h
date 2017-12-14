#ifndef STINGRAYKIT_METAPROGRAMMING_PARAMPASSINGTYPE_H
#define STINGRAYKIT_METAPROGRAMMING_PARAMPASSINGTYPE_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/metaprogramming/EnableIf.h>
#include <stingraykit/metaprogramming/TypeTraits.h>

namespace stingray
{

	namespace Detail_ParamPassingType
	{

		template < typename T, typename Enabler = void > struct PassByRef									: TrueType { };
		template < typename T > struct PassByRef<T, typename EnableIf<IsBuiltin<T>::Value, void>::ValueT>	: FalseType { };
		template < typename T > struct PassByRef<T, typename EnableIf<IsPointer<T>::Value, void>::ValueT>	: FalseType { };
		template < typename T > struct PassByRef<T, typename EnableIf<IsEnumClass<T>::Value, void>::ValueT>	: FalseType { };

	}


	template < typename T >
	class GetParamPassingType
	{
		typedef typename RemoveReference<typename RemoveConst<T>::ValueT>::ValueT RawType;
		typedef typename If<Detail_ParamPassingType::PassByRef<T>::Value, typename AddConstReference<RawType>::ValueT, RawType>::ValueT ConstPassingType;

	public:
		typedef typename If<IsNonConstReference<T>::Value, typename AddReference<RawType>::ValueT, ConstPassingType>::ValueT ValueT;
	};

}

#endif
