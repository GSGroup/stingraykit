#ifndef STINGRAYKIT_THREAD_DISABLEINTERRUPTIONPOINTS_H
#define STINGRAYKIT_THREAD_DISABLEINTERRUPTIONPOINTS_H

#include <stingray/toolkit/thread/Thread.h>
#include <stingray/toolkit/function/function_info.h>

namespace stingray {

	/**
	 * @addtogroup toolkit_threads
	 * @{
	 */

	template<typename FuncType, typename ModifierToken, size_t ParamsNum>
	struct ExecutionContextModifier;

	template<typename FuncType, typename ModifierToken>
	struct ExecutionContextModifier<FuncType, ModifierToken, 0> : public function_info<FuncType>
	{
		typedef typename function_info<FuncType>::RetType		RetType;
		typedef typename function_info<FuncType>::ParamTypes	ParamTypes;
	private:
		FuncType	_func;

	public:
		ExecutionContextModifier(const FuncType& func) : _func(func)
		{}
		RetType operator() () const
		{
			ModifierToken token;
			return _func();
		}
	};

#ifndef DOXYGEN_PREPROCESSOR
#define TY typename
#define DETAIL_DECLARE_EXECUTION_CONTEXT_MODIFIER(ParamsNum_, ParamsTypes_, ParamsDecl_, ParamsUsage_) \
	template<typename FuncType, typename ModifierToken> \
	struct ExecutionContextModifier<FuncType, ModifierToken, ParamsNum_> : public function_info<FuncType> \
	{ \
		typedef typename function_info<FuncType>::RetType 		RetType; \
		typedef typename function_info<FuncType>::ParamTypes	ParamTypes; \
	private:  \
		FuncType	_func; \
	public: \
		ExecutionContextModifier(const FuncType& func) : _func(func) \
		{} \
		template<ParamsTypes_> \
		RetType operator() (ParamsDecl_) const \
		{ \
			ModifierToken token; \
			return _func(ParamsUsage_); \
		} \
	};

	DETAIL_DECLARE_EXECUTION_CONTEXT_MODIFIER(1, MK_PARAM(TY T1), MK_PARAM(T1 p1), MK_PARAM(p1))
	DETAIL_DECLARE_EXECUTION_CONTEXT_MODIFIER(2, MK_PARAM(TY T1, TY T2), MK_PARAM(T1 p1, T2 p2), MK_PARAM(p1, p2))
	DETAIL_DECLARE_EXECUTION_CONTEXT_MODIFIER(3, MK_PARAM(TY T1, TY T2, TY T3), MK_PARAM(T1 p1, T2 p2, T3 p3), MK_PARAM(p1, p2, p3))
	DETAIL_DECLARE_EXECUTION_CONTEXT_MODIFIER(4, MK_PARAM(TY T1, TY T2, TY T3, TY T4), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4), MK_PARAM(p1, p2, p3, p4))
	DETAIL_DECLARE_EXECUTION_CONTEXT_MODIFIER(5, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5), MK_PARAM(p1, p2, p3, p4, p5))
	DETAIL_DECLARE_EXECUTION_CONTEXT_MODIFIER(6, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6), MK_PARAM(p1, p2, p3, p4, p5, p6))
	DETAIL_DECLARE_EXECUTION_CONTEXT_MODIFIER(7, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7), MK_PARAM(p1, p2, p3, p4, p5, p6, p7))
	DETAIL_DECLARE_EXECUTION_CONTEXT_MODIFIER(8, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8))
	DETAIL_DECLARE_EXECUTION_CONTEXT_MODIFIER(9, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8, T9 p9), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9))
	DETAIL_DECLARE_EXECUTION_CONTEXT_MODIFIER(10, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8, T9 p9, T10 p10), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10))

#undef DETAIL_DECLARE_EXECUTION_CONTEXT_MODIFIER
#undef TY
#endif

	struct DisableInterruptionPointsToken : public EnableInterruptionPoints
	{
		DisableInterruptionPointsToken() : EnableInterruptionPoints(false) {}
	};

	template<typename FuncType>
	ExecutionContextModifier<FuncType, DisableInterruptionPointsToken, GetTypeListLength<typename function_info<FuncType>::ParamTypes>::Value> DisableInterruptionPoints(const FuncType& func)
	{ return ExecutionContextModifier<FuncType, DisableInterruptionPointsToken, GetTypeListLength<typename function_info<FuncType>::ParamTypes>::Value>(func); }

	/** @} */

}


#endif

