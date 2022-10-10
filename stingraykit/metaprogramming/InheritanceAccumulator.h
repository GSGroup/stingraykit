#ifndef STINGRAYKIT_METAPROGRAMMING_INHERITANCEACCUMULATOR_H
#define STINGRAYKIT_METAPROGRAMMING_INHERITANCEACCUMULATOR_H

// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/metaprogramming/TypeList.h>
#include <stingraykit/Macro.h>

namespace stingray
{

	namespace Detail
	{

#define DETAIL_STINGRAYKIT_IA_CTOR(N_) \
	template<STINGRAYKIT_REPEAT(N_, STINGRAYKIT_TEMPLATE_PARAM_DECL, T)> \
	InheritanceAccumulatorImpl(STINGRAYKIT_REPEAT(N_, STINGRAYKIT_FUNCTION_PARAM_DECL, T)) : \
		Ts(STINGRAYKIT_REPEAT(N_, STINGRAYKIT_FUNCTION_PARAM_USAGE, ~))... { } \
	template<STINGRAYKIT_REPEAT(N_, STINGRAYKIT_TEMPLATE_PARAM_DECL, T)> \
	InheritanceAccumulatorImpl(STINGRAYKIT_REPEAT(N_, STINGRAYKIT_FUNCTION_PARAM_DECL_BYREF, T)) : \
		Ts(STINGRAYKIT_REPEAT(N_, STINGRAYKIT_FUNCTION_PARAM_USAGE, ~))... { }

		template < typename... Ts >
		struct InheritanceAccumulatorImpl : public Ts...
		{
			InheritanceAccumulatorImpl() { }

#if defined(__GNUC__) && !defined(__clang__)
			//GCC has bug: STB-25620, https://gcc.gnu.org/bugzilla/show_bug.cgi?id=88580
			DETAIL_STINGRAYKIT_IA_CTOR(1);
			DETAIL_STINGRAYKIT_IA_CTOR(2);
			DETAIL_STINGRAYKIT_IA_CTOR(3);
			DETAIL_STINGRAYKIT_IA_CTOR(4);
			DETAIL_STINGRAYKIT_IA_CTOR(5);
#else
			template < typename U0, typename... Us >
			InheritanceAccumulatorImpl(const U0& p0, const Us&... args) : Ts(p0, args...)...
			{ }

			template < typename U0, typename... Us >
			InheritanceAccumulatorImpl(U0& p0, Us&... args) : Ts(p0, args...)...
			{ }
#endif
		};

#undef DETAIL_STINGRAYKIT_IA_CTOR

		template < typename TypeList, typename... Ts >
		struct InheritanceAccumulatorCreator
		{
			typedef typename InheritanceAccumulatorCreator<typename TypeList::Next, Ts..., typename TypeList::ValueT>::ValueT ValueT;
		};

		template < typename... Ts >
		struct InheritanceAccumulatorCreator<TypeListEndNode, Ts...>
		{
			typedef Detail::InheritanceAccumulatorImpl<Ts...> ValueT;
		};

	}


	template < typename TypeList >
	struct InheritanceAccumulator
	{
		typedef typename Detail::InheritanceAccumulatorCreator<TypeList>::ValueT ValueT;
	};

}

#endif
