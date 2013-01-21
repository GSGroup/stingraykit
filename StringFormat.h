#ifndef STINGRAY_TOOLKIT_STRINGFORMAT_H
#define STINGRAY_TOOLKIT_STRINGFORMAT_H

#include <stingray/toolkit/StringUtils.h>

namespace stingray {

	namespace Detail
	{
		template<typename TupleParams>
		struct TupleToStringHelper
		{
		public:
			static std::string ItemToString(const Tuple<TupleParams>& tuple, size_t index)
			{
				if (index == 0)
					return ToString(tuple.GetHead());
				return TupleToStringHelper<typename TupleParams::Next>::ItemToString(tuple.GetTail(), index - 1);
			}
		};

		template<>
		struct TupleToStringHelper<TypeListEndNode>
		{
			static std::string ItemToString(const Tuple<TypeListEndNode>& tuple, size_t index)
			{ TOOLKIT_THROW("Tuple item index mismatch!"); }
		};
	}

	template<typename TupleParams>
	static std::string StringFormat(const std::string& format, const Tuple<TupleParams>& params)
	{
		std::vector<std::string> substrings;
		Split(format, "%", substrings);

		TOOLKIT_CHECK((substrings.size() % 2) == 1, "Format mismatch: no corresponding %");
		std::string result;
		for (size_t i = 0; i != substrings.size(); ++i)
			if (i % 2)
			{
				if (substrings[i].empty())
					result += "%";
				else
				{
					size_t pos = substrings[i].find('$');
					size_t index = FromString<size_t>(substrings[i].substr(0, pos));
					TOOLKIT_CHECK(index > 0, "Format mismatch: parameters indices start from 1!");
					size_t width = (pos == std::string::npos) ? 0 : FromString<size_t>(substrings[i].substr(pos + 1));
					std::string item_str = Detail::TupleToStringHelper<TupleParams>::ItemToString(params, index - 1);
					if (item_str.size() < width)
						result += std::string(width - item_str.size(), '0') + item_str;
					else
						result += item_str;
				}
			}
			else
				result += substrings[i];

		return result;
	}

#define TY typename
#define P_(N) const T##N & p##N

#define DETAIL_TOOLKIT_DECLARE_STRINGFORMAT(N_, TypesDecl_, TypesUsage_, ParamsDecl_, ParamsUsage_) \
	template < TypesDecl_ > \
	static std::string StringFormat(const std::string& format, ParamsDecl_) \
	{ \
		typedef typename TypeListTransform<TypeList_##N_<TypesUsage_>, GetConstReferenceType>::ValueT TupleParams; \
		return StringFormat(format, Tuple<TupleParams>(ParamsUsage_)); \
	}

	DETAIL_TOOLKIT_DECLARE_STRINGFORMAT(1, MK_PARAM(TY T1), MK_PARAM(T1), MK_PARAM(P_(1)), MK_PARAM(p1));
	DETAIL_TOOLKIT_DECLARE_STRINGFORMAT(2, MK_PARAM(TY T1, TY T2), MK_PARAM(T1, T2), MK_PARAM(P_(1), P_(2)), MK_PARAM(p1, p2));
	DETAIL_TOOLKIT_DECLARE_STRINGFORMAT(3, MK_PARAM(TY T1, TY T2, TY T3), MK_PARAM(T1, T2, T3), MK_PARAM(P_(1), P_(2), P_(3)), MK_PARAM(p1, p2, p3));
	DETAIL_TOOLKIT_DECLARE_STRINGFORMAT(4, MK_PARAM(TY T1, TY T2, TY T3, TY T4), MK_PARAM(T1, T2, T3, T4), MK_PARAM(P_(1), P_(2), P_(3), P_(4)), MK_PARAM(p1, p2, p3, p4));
	DETAIL_TOOLKIT_DECLARE_STRINGFORMAT(5, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5), MK_PARAM(T1, T2, T3, T4, T5), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5)), MK_PARAM(p1, p2, p3, p4, p5));
	DETAIL_TOOLKIT_DECLARE_STRINGFORMAT(6, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6), MK_PARAM(T1, T2, T3, T4, T5, T6), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6)), MK_PARAM(p1, p2, p3, p4, p5, p6));
	DETAIL_TOOLKIT_DECLARE_STRINGFORMAT(7, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7), MK_PARAM(T1, T2, T3, T4, T5, T6, T7), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7));
	DETAIL_TOOLKIT_DECLARE_STRINGFORMAT(8, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8));
	DETAIL_TOOLKIT_DECLARE_STRINGFORMAT(9, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9));
	DETAIL_TOOLKIT_DECLARE_STRINGFORMAT(10, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10));
	DETAIL_TOOLKIT_DECLARE_STRINGFORMAT(11, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11));
	DETAIL_TOOLKIT_DECLARE_STRINGFORMAT(12, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12));
	DETAIL_TOOLKIT_DECLARE_STRINGFORMAT(13, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13));
	DETAIL_TOOLKIT_DECLARE_STRINGFORMAT(14, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13), P_(14)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14));
	DETAIL_TOOLKIT_DECLARE_STRINGFORMAT(15, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13), P_(14), P_(15)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15));
	DETAIL_TOOLKIT_DECLARE_STRINGFORMAT(16, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15, TY T16), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13), P_(14), P_(15), P_(16)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16));
	DETAIL_TOOLKIT_DECLARE_STRINGFORMAT(17, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15, TY T16, TY T17), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13), P_(14), P_(15), P_(16), P_(17)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17));
	DETAIL_TOOLKIT_DECLARE_STRINGFORMAT(18, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15, TY T16, TY T17, TY T18), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13), P_(14), P_(15), P_(16), P_(17), P_(18)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18));
	DETAIL_TOOLKIT_DECLARE_STRINGFORMAT(19, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15, TY T16, TY T17, TY T18, TY T19), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13), P_(14), P_(15), P_(16), P_(17), P_(18), P_(19)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19));
	DETAIL_TOOLKIT_DECLARE_STRINGFORMAT(20, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15, TY T16, TY T17, TY T18, TY T19, TY T20), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13), P_(14), P_(15), P_(16), P_(17), P_(18), P_(19), P_(20)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20));

#undef DETAIL_TOOLKIT_DECLARE_STRINGFORMAT
#undef P_
#undef TY


}


#endif

