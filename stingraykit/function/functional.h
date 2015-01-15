#ifndef STINGRAYKIT_FUNCTION_FUNCTIONAL_H
#define STINGRAYKIT_FUNCTION_FUNCTIONAL_H


#include <stingraykit/function/function.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

	namespace Detail
	{
		template < typename F, size_t ParamCount = GetTypeListLength<typename function_info<F>::ParamTypes>::Value >
		class NotFunc;

#define DETAIL_STINGRAYKIT_DECLARE_NOTFUNC(ParamsCount_, ParamTypes_, ParamsDecl_, ParamsUsage_) \
			template < typename F > \
			class NotFunc<F, ParamsCount_> : public function_info<bool, typename function_info<F>::ParamTypes> \
			{ \
				F	_f; \
			public: \
				NotFunc(const F& f) : _f(f) { } \
				template < ParamTypes_ > bool operator() (ParamsDecl_) const { return !_f(ParamsUsage_); } \
			}

//#define P_(N_) typename GetParamPassingType<typename GetTypeListItem<typename function_info<F>::ParamTypes, N_ - 1>::ValueT>::ValueT p##N_
#define P_(N_) const T##N_& p##N_
#define TY typename

		DETAIL_STINGRAYKIT_DECLARE_NOTFUNC(1, MK_PARAM(TY T1), MK_PARAM(P_(1)), MK_PARAM(p1));
		DETAIL_STINGRAYKIT_DECLARE_NOTFUNC(2, MK_PARAM(TY T1, TY T2), MK_PARAM(P_(1), P_(2)), MK_PARAM(p1, p2));
		DETAIL_STINGRAYKIT_DECLARE_NOTFUNC(3, MK_PARAM(TY T1, TY T2, TY T3), MK_PARAM(P_(1), P_(2), P_(3)), MK_PARAM(p1, p2, p3));

#undef TY
#undef P_
#undef DETAIL_STINGRAYKIT_DECLARE_NOTFUNC
	}

	template < typename F >
	Detail::NotFunc<F> not_(const F& f) { return Detail::NotFunc<F>(f); }

	/** @} */

}


#endif
