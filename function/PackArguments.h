#ifndef STINGRAYKIT_FUNCTION_PACKARGUMENTS_H
#define STINGRAYKIT_FUNCTION_PACKARGUMENTS_H

#include <stingray/toolkit/function/function.h>
#include <stingray/toolkit/Tuple.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

	namespace Detail
	{

		template < size_t ArgumentCount, typename Signature>
		class ArgumentPacker;

#define DETAIL_STINGRAYKIT_DECLARE_ARGUMENT_PACKER(N, Args_, Usage_) \
		\
		template <typename Signature> class ArgumentPacker<N, Signature> : public function_info<Signature> \
		{ \
		public: \
			typedef typename function_info<Signature>::RetType		RetType; \
			typedef typename function_info<Signature>::ParamTypes	ParamTypes; \
			typedef Tuple<ParamTypes>								TupleType; \
			typedef function<RetType (const TupleType &)>			FuncType; \
		private: \
			FuncType _func; \
		public: \
			ArgumentPacker(const FuncType &func) : _func(func) {} \
			\
			RetType operator() (Args_) const \
			{ return _func(TupleType(Usage_)); } \
		} \


#define P(N) const typename GetTypeListItem<ParamTypes, N - 1>::ValueT & p##N

		DETAIL_STINGRAYKIT_DECLARE_ARGUMENT_PACKER(0, /**/, /**/);
		DETAIL_STINGRAYKIT_DECLARE_ARGUMENT_PACKER(1, MK_PARAM(P(1)), MK_PARAM(p1));
		DETAIL_STINGRAYKIT_DECLARE_ARGUMENT_PACKER(2, MK_PARAM(P(1), P(2)), MK_PARAM(p1, p2));
		DETAIL_STINGRAYKIT_DECLARE_ARGUMENT_PACKER(3, MK_PARAM(P(1), P(2), P(3)), MK_PARAM(p1, p2, p3));
		DETAIL_STINGRAYKIT_DECLARE_ARGUMENT_PACKER(4, MK_PARAM(P(1), P(2), P(3), P(4)), MK_PARAM(p1, p2, p3, p4));
		DETAIL_STINGRAYKIT_DECLARE_ARGUMENT_PACKER(5, MK_PARAM(P(1), P(2), P(3), P(4), P(5)), MK_PARAM(p1, p2, p3, p4, p5));
		DETAIL_STINGRAYKIT_DECLARE_ARGUMENT_PACKER(6, MK_PARAM(P(1), P(2), P(3), P(4), P(5), P(6)), MK_PARAM(p1, p2, p3, p4, p5, p6));
		DETAIL_STINGRAYKIT_DECLARE_ARGUMENT_PACKER(7, MK_PARAM(P(1), P(2), P(3), P(4), P(5), P(6), P(7)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7));
		DETAIL_STINGRAYKIT_DECLARE_ARGUMENT_PACKER(8, MK_PARAM(P(1), P(2), P(3), P(4), P(5), P(6), P(7), P(8)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8));
		DETAIL_STINGRAYKIT_DECLARE_ARGUMENT_PACKER(9, MK_PARAM(P(1), P(2), P(3), P(4), P(5), P(6), P(7), P(8), P(9)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9));
		DETAIL_STINGRAYKIT_DECLARE_ARGUMENT_PACKER(10, MK_PARAM(P(1), P(2), P(3), P(4), P(5), P(6), P(7), P(8), P(9), P(10)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10));

#undef P
#undef DETAIL_STINGRAYKIT_DECLARE_ARGUMENT_PACKER
	}

	template<typename Signature>
	function<Signature> PackArguments(const function<void (const Tuple<typename function_info<Signature>::ParamTypes> &) > &func)
	{
		typedef const Tuple<typename function_info<Signature>::ParamTypes> TupleType;
		return function<Signature>(Detail::ArgumentPacker<TupleType::Size, Signature>(func));
	}

	/** @} */

}

#endif
