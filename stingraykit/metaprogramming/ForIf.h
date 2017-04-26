#ifndef STINGRAYKIT_METAPROGRAMMING_FORIF_H
#define STINGRAYKIT_METAPROGRAMMING_FORIF_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/Macro.h>

namespace stingray
{

#define TY typename

	template < unsigned Count, template <int> class FunctorClass, int Start = 0 >
	struct ForIf
	{
		static bool Do()
		{
			bool success = FunctorClass<Start>::Call();
			return success && ForIf<Count - 1, FunctorClass, Start + 1>::Do();
		}

#define DETAIL_STINGRAYKIT_DECLARE_FOR_IF_DO(TypesDecl_, TypesUsage_, ParamsDecl_, ParamsUsage_) \
		template < TypesDecl_ > \
		static bool Do(ParamsDecl_) \
		{ \
			bool success = FunctorClass<Start>::Call(ParamsUsage_); \
			return success && ForIf<Count - 1, FunctorClass, Start + 1>::template Do<TypesUsage_>(ParamsUsage_); \
		}

		DETAIL_STINGRAYKIT_DECLARE_FOR_IF_DO(MK_PARAM(TY T1), MK_PARAM(T1), MK_PARAM(const T1& p1), MK_PARAM(p1));
		DETAIL_STINGRAYKIT_DECLARE_FOR_IF_DO(MK_PARAM(TY T1, TY T2), MK_PARAM(T1, T2), MK_PARAM(const T1& p1, const T2& p2), MK_PARAM(p1, p2));
		DETAIL_STINGRAYKIT_DECLARE_FOR_IF_DO(MK_PARAM(TY T1, TY T2, TY T3), MK_PARAM(T1, T2, T3), MK_PARAM(const T1& p1, const T2& p2, const T3& p3), MK_PARAM(p1, p2, p3));

#undef DETAIL_STINGRAYKIT_DECLARE_FOR_IF_DO
	};

	template < template <int> class FunctorClass, int Start >
	struct ForIf<0, FunctorClass, Start>
	{
		static bool Do() { return true; }

		template <TY T1> static bool Do(const T1&) { return true; }
		template <TY T1, TY T2> static bool Do(const T1&, const T2&) { return true; }
		template <TY T1, TY T2, TY T3> static bool Do(const T1&, const T2&, const T3&) { return true; }
	};

#undef TY

}

#endif
