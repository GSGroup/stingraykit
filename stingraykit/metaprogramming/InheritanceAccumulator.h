#ifndef STINGRAYKIT_METAPROGRAMMING_INHERITANCEACCUMULATOR_H
#define STINGRAYKIT_METAPROGRAMMING_INHERITANCEACCUMULATOR_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/metaprogramming/TypeList.h>

namespace stingray
{

	template < typename TypeList, typename EndNode >
	class InheritanceAccumulator
	{
		template < typename Current, typename Result >
		struct AccumulateFunc
		{
			struct ValueT : public Current, public Result
			{
#define DETAIL_STINGRAYKIT_DECLARE_VALUET_CTOR(N_, UserArg_) \
				STINGRAYKIT_INSERT_IF(N_, template <) STINGRAYKIT_REPEAT(N_, STINGRAYKIT_TEMPLATE_PARAM_DECL, T) STINGRAYKIT_INSERT_IF(N_, >) \
				ValueT(STINGRAYKIT_REPEAT(N_, STINGRAYKIT_FUNCTION_PARAM_DECL, T)) : Current(STINGRAYKIT_REPEAT(N_, STINGRAYKIT_FUNCTION_PARAM_USAGE, T)), Result(STINGRAYKIT_REPEAT(N_, STINGRAYKIT_FUNCTION_PARAM_USAGE, T)) { }
				STINGRAYKIT_REPEAT_NESTING_2(10, DETAIL_STINGRAYKIT_DECLARE_VALUET_CTOR, ~)
#undef DETAIL_STINGRAYKIT_DECLARE_VALUET_CTOR

			};
		};

	public:
		typedef typename TypeListAccumulate<typename ToTypeList<TypeList>::ValueT, AccumulateFunc, EndNode>::ValueT ValueT;
	};

}

#endif
