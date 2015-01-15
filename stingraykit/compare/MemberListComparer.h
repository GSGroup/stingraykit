#ifndef STINGRAYKIT_COMPARE_MEMBERLISTCOMPARER_H
#define STINGRAYKIT_COMPARE_MEMBERLISTCOMPARER_H

#include <stingraykit/compare/MemberToValueComparer.h>
#include <stingraykit/Tuple.h>
#include <stingraykit/compare/comparers.h>
#include <stingraykit/shared_ptr.h>

namespace stingray
{

	namespace Detail
	{
		template <typename MemberPointerTuple>
		struct MemberListComparerImpl
		{
			template <typename ClassType, typename MemberPointerType>
			static int CompareMember(const ClassType &lhs, const ClassType &rhs, const MemberPointerType &pointer)
			{
				typedef MemberExtractor<MemberPointerType> Extractor;
				return comparers::Cmp()(Extractor::GetValue(lhs, pointer), Extractor::GetValue(rhs, pointer));
			}

			template <typename ClassType, typename MemberPointerT, typename ComparerT>
			static int CompareMember(const ClassType &lhs, const ClassType &rhs, const CustomMemberComparerWrapper<MemberPointerT, ComparerT> &comparer)
			{ return comparer.Compare(lhs, rhs); }

			template <typename ClassType>
			static int Do(const ClassType &lhs, const ClassType &rhs, const MemberPointerTuple &tuple)
			{
				int result = CompareMember(lhs, rhs, tuple.GetHead());
				if (result == 0)
					return MemberListComparerImpl<typename MemberPointerTuple::Tail>::Do(lhs, rhs, tuple.GetTail());
				return result;
			}
		};


		template < >
		struct MemberListComparerImpl<Tuple<TypeListEndNode> >
		{
			template <typename ClassType>
			static int Do(const ClassType &lhs, const ClassType &rhs, const Tuple<TypeListEndNode> &tuple)
			{ return 0; }
		};
	}


	template <typename MemberPointerTuple>
	struct MemberListComparer : public comparers::CmpComparerBase<MemberListComparer<MemberPointerTuple> >
	{
	private:
		MemberPointerTuple _memberPointerList;

	public:
		MemberListComparer(const MemberPointerTuple& memberPointerList) : _memberPointerList(memberPointerList)
		{ }

		template <typename T>
		int DoCompare(const T& lhs, const T& rhs) const
		{ return Detail::MemberListComparerImpl<MemberPointerTuple>::Do(lhs, rhs, _memberPointerList); }
	};


#define TY typename
#define P_(N) const T##N& p##N
#define DETAIL_STINGRAYKIT_DECLARE_MAKEMEMBERLISTCOMPARER(N_, TypesDecl_, TypesUsage_, ParamsDecl_, ParamsUsage_) \
	template < TypesDecl_ > \
	MemberListComparer<Tuple<TypeList_##N_<TypesUsage_> > > CompareMembersCmp(ParamsDecl_) \
	{ return MemberListComparer<Tuple<TypeList_##N_<TypesUsage_> > >(Tuple<TypeList_##N_<TypesUsage_> >(ParamsUsage_)); } \
	template <TypesDecl_> \
	comparers::CmpToLess<MemberListComparer<Tuple<TypeList_##N_<TypesUsage_> > > > CompareMembersLess(ParamsDecl_) \
	{ return comparers::CmpToLess<MemberListComparer<Tuple<TypeList_##N_<TypesUsage_> > > >(Tuple<TypeList_##N_<TypesUsage_> >(ParamsUsage_)); } \
	template <TypesDecl_> \
	comparers::CmpToGreater<MemberListComparer<Tuple<TypeList_##N_<TypesUsage_> > > > CompareMembersGreater(ParamsDecl_) \
	{ return comparers::CmpToGreater<MemberListComparer<Tuple<TypeList_##N_<TypesUsage_> > > >(Tuple<TypeList_##N_<TypesUsage_> >(ParamsUsage_)); } \
	template <TypesDecl_> \
	comparers::CmpToEquals<MemberListComparer<Tuple<TypeList_##N_<TypesUsage_> > > > CompareMembersEquals(ParamsDecl_) \
	{ return comparers::CmpToEquals<MemberListComparer<Tuple<TypeList_##N_<TypesUsage_> > > >(Tuple<TypeList_##N_<TypesUsage_> >(ParamsUsage_)); }


	DETAIL_STINGRAYKIT_DECLARE_MAKEMEMBERLISTCOMPARER(1, MK_PARAM(TY T1), MK_PARAM(T1), MK_PARAM(P_(1)), MK_PARAM(p1));
	DETAIL_STINGRAYKIT_DECLARE_MAKEMEMBERLISTCOMPARER(2, MK_PARAM(TY T1, TY T2), MK_PARAM(T1, T2), MK_PARAM(P_(1), P_(2)), MK_PARAM(p1, p2));
	DETAIL_STINGRAYKIT_DECLARE_MAKEMEMBERLISTCOMPARER(3, MK_PARAM(TY T1, TY T2, TY T3), MK_PARAM(T1, T2, T3), MK_PARAM(P_(1), P_(2), P_(3)), MK_PARAM(p1, p2, p3));
	DETAIL_STINGRAYKIT_DECLARE_MAKEMEMBERLISTCOMPARER(4, MK_PARAM(TY T1, TY T2, TY T3, TY T4), MK_PARAM(T1, T2, T3, T4), MK_PARAM(P_(1), P_(2), P_(3), P_(4)), MK_PARAM(p1, p2, p3, p4));
	DETAIL_STINGRAYKIT_DECLARE_MAKEMEMBERLISTCOMPARER(5, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5), MK_PARAM(T1, T2, T3, T4, T5), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5)), MK_PARAM(p1, p2, p3, p4, p5));
	DETAIL_STINGRAYKIT_DECLARE_MAKEMEMBERLISTCOMPARER(6, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6), MK_PARAM(T1, T2, T3, T4, T5, T6), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6)), MK_PARAM(p1, p2, p3, p4, p5, p6));
	DETAIL_STINGRAYKIT_DECLARE_MAKEMEMBERLISTCOMPARER(7, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7), MK_PARAM(T1, T2, T3, T4, T5, T6, T7), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7));
	DETAIL_STINGRAYKIT_DECLARE_MAKEMEMBERLISTCOMPARER(8, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8));
	DETAIL_STINGRAYKIT_DECLARE_MAKEMEMBERLISTCOMPARER(9, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9));
	DETAIL_STINGRAYKIT_DECLARE_MAKEMEMBERLISTCOMPARER(10, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10));
	DETAIL_STINGRAYKIT_DECLARE_MAKEMEMBERLISTCOMPARER(11, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11));
	DETAIL_STINGRAYKIT_DECLARE_MAKEMEMBERLISTCOMPARER(12, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12));
	DETAIL_STINGRAYKIT_DECLARE_MAKEMEMBERLISTCOMPARER(13, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13));
	DETAIL_STINGRAYKIT_DECLARE_MAKEMEMBERLISTCOMPARER(14, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13), P_(14)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14));
	DETAIL_STINGRAYKIT_DECLARE_MAKEMEMBERLISTCOMPARER(15, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13), P_(14), P_(15)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15));
	DETAIL_STINGRAYKIT_DECLARE_MAKEMEMBERLISTCOMPARER(16, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15, TY T16), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13), P_(14), P_(15), P_(16)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16));
	DETAIL_STINGRAYKIT_DECLARE_MAKEMEMBERLISTCOMPARER(17, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15, TY T16, TY T17), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13), P_(14), P_(15), P_(16), P_(17)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17));
	DETAIL_STINGRAYKIT_DECLARE_MAKEMEMBERLISTCOMPARER(18, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15, TY T16, TY T17, TY T18), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13), P_(14), P_(15), P_(16), P_(17), P_(18)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18));
	DETAIL_STINGRAYKIT_DECLARE_MAKEMEMBERLISTCOMPARER(19, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15, TY T16, TY T17, TY T18, TY T19), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13), P_(14), P_(15), P_(16), P_(17), P_(18), P_(19)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19));
	DETAIL_STINGRAYKIT_DECLARE_MAKEMEMBERLISTCOMPARER(20, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15, TY T16, TY T17, TY T18, TY T19, TY T20), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13), P_(14), P_(15), P_(16), P_(17), P_(18), P_(19), P_(20)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20));

#undef DETAIL_STINGRAYKIT_DECLARE_MAKEMEMBERLISTCOMPARER
#undef P_
#undef TY

}

#endif
