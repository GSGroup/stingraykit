#ifndef STINGRAYKIT_COLLECTION_MAKECOLLECTION_H
#define STINGRAYKIT_COLLECTION_MAKECOLLECTION_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/metaprogramming/EnableIf.h>
#include <stingraykit/metaprogramming/TypeTraits.h>

#include <vector>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	namespace Detail
	{

		struct EmptyVectorProxy
		{
			template < typename T >
			operator std::vector<T> () const
			{ return { }; }
		};

		template < typename T, typename... Us >
		struct IsSameTypes
		{
			template < typename U >
			using Predicate = IsSame<typename Decay<T>::ValueT, typename Decay<U>::ValueT>;

			static const bool Value = TypeListAllOf<TypeList<Us...>, Predicate>::Value;
		};

	}


	inline Detail::EmptyVectorProxy make_vector()
	{ return Detail::EmptyVectorProxy(); }

	template < typename T0, typename... Ts, typename EnableIf<Detail::IsSameTypes<T0, Ts...>::Value, int>::ValueT = 0 >
	std::vector<typename Decay<T0>::ValueT> make_vector(T0&& p0, Ts&&... args)
	{ return { std::forward<T0>(p0), std::forward<Ts>(args)... }; }

	/** @} */

}

#endif
