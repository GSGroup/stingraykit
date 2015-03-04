#ifndef STINGRAYKIT_METAPROGRAMMING_FOR_H
#define STINGRAYKIT_METAPROGRAMMING_FOR_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/Macro.h>

namespace stingray
{

	template < unsigned Count, template <int> class FunctorClass, int Start = 0 >
	struct For
	{
#define DETAIL_STINGRAYKIT_DECLARE_FOR_DO(N_, UserArg_) \
		STINGRAYKIT_INSERT_IF(N_, template <) STINGRAYKIT_REPEAT(N_, STINGRAYKIT_TEMPLATE_PARAM_DECL, T) STINGRAYKIT_INSERT_IF(N_, >) \
		static void Do(STINGRAYKIT_REPEAT(N_, STINGRAYKIT_FUNCTION_PARAM_DECL, T)) \
		{ \
			FunctorClass<Start>::Call(STINGRAYKIT_REPEAT(N_, STINGRAYKIT_FUNCTION_PARAM_USAGE, T)); \
			For<Count - 1, FunctorClass, Start + 1>:: STINGRAYKIT_INSERT_IF(N_, template) Do STINGRAYKIT_INSERT_IF(N_, <) STINGRAYKIT_REPEAT(N_, STINGRAYKIT_TEMPLATE_PARAM_USAGE, T) STINGRAYKIT_INSERT_IF(N_, >) (STINGRAYKIT_REPEAT(N_, STINGRAYKIT_FUNCTION_PARAM_USAGE, T)); \
		}

		STINGRAYKIT_REPEAT_NESTING_2(10, DETAIL_STINGRAYKIT_DECLARE_FOR_DO, ~)

#undef DETAIL_STINGRAYKIT_DECLARE_FOR_DO
	};

	template < template <int> class FunctorClass, int Start >
	struct For<0, FunctorClass, Start>
	{
#define DETAIL_STINGRAYKIT_DECLARE_FOR_DO(N_, UserArg_) \
		STINGRAYKIT_INSERT_IF(N_, template <) STINGRAYKIT_REPEAT(N_, STINGRAYKIT_TEMPLATE_PARAM_DECL, T) STINGRAYKIT_INSERT_IF(N_, >) \
		static void Do(STINGRAYKIT_REPEAT(N_, STINGRAYKIT_FUNCTION_PARAM_DECL, T)) \
		{ }

		STINGRAYKIT_REPEAT_NESTING_2(10, DETAIL_STINGRAYKIT_DECLARE_FOR_DO, ~)

#undef DETAIL_STINGRAYKIT_DECLARE_FOR_DO
	};

}

#endif
