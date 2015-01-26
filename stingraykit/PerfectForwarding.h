#ifndef STINGRAYKIT_PERFECTFORWARDING_H
#define STINGRAYKIT_PERFECTFORWARDING_H


#include <stingraykit/Macro.h>

namespace stingray
{

#define DETAIL_PERFECT_FORWARDING_OP_PARAM_TYPE(ParamIndex_, ParamVariantId_) \
	STINGRAYKIT_COMMA_IF(ParamIndex_) STINGRAYKIT_INSERT_IF(STINGRAYKIT_BITWISE_AND(ParamVariantId_, STINGRAYKIT_POW(2, ParamIndex_)), const) STINGRAYKIT_CAT(T, ParamIndex_)&

#define DETAIL_PERFECT_FORWARDING_OP_PARAM_DECL(ParamIndex_, ParamVariantId_) \
	DETAIL_PERFECT_FORWARDING_OP_PARAM_TYPE(ParamIndex_, ParamVariantId_) STINGRAYKIT_CAT(p, ParamIndex_)

#define DETAIL_PERFECT_FORWARDING(N_, ParamsCount_, RetType_, FromFunction_, ToFunction_) \
	template< STINGRAYKIT_REPEAT_NESTING_2(ParamsCount_, STINGRAYKIT_TEMPLATE_PARAM_DECL, T) > \
	RetType_ FromFunction_ (STINGRAYKIT_REPEAT_NESTING_2(ParamsCount_, DETAIL_PERFECT_FORWARDING_OP_PARAM_DECL, N_)) const \
	{ \
		typedef typename TypeList<STINGRAYKIT_REPEAT_NESTING_2(ParamsCount_, DETAIL_PERFECT_FORWARDING_OP_PARAM_TYPE, N_)>::type PT; \
		Tuple<PT> p(STINGRAYKIT_REPEAT_NESTING_2(ParamsCount_, STINGRAYKIT_FUNCTION_PARAM_USAGE, ~)); \
		return ToFunction_<PT>(p); \
	}

#define STINGRAYKIT_PERFECT_FORWARDING(RetType_, FromFunction_, ToFunction_) \
	RetType_ FromFunction_ () const \
	{ \
		typedef TypeList<>::type PT; \
		Tuple<PT> p; \
		return ToFunction_<PT>(p); \
	} \
	STINGRAYKIT_REPEAT( 2, DETAIL_PERFECT_FORWARDING, 1, RetType_, FromFunction_, ToFunction_) \
	STINGRAYKIT_REPEAT( 4, DETAIL_PERFECT_FORWARDING, 2, RetType_, FromFunction_, ToFunction_) \
	STINGRAYKIT_REPEAT( 8, DETAIL_PERFECT_FORWARDING, 3, RetType_, FromFunction_, ToFunction_) \
	STINGRAYKIT_REPEAT(16, DETAIL_PERFECT_FORWARDING, 4, RetType_, FromFunction_, ToFunction_) \
	STINGRAYKIT_REPEAT(32, DETAIL_PERFECT_FORWARDING, 5, RetType_, FromFunction_, ToFunction_) \
	STINGRAYKIT_REPEAT(64, DETAIL_PERFECT_FORWARDING, 6, RetType_, FromFunction_, ToFunction_)

}

#endif
