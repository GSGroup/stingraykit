#ifndef STINGRAYKIT_METAPROGRAMMING_FORIF_H
#define STINGRAYKIT_METAPROGRAMMING_FORIF_H

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
