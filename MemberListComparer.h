#ifndef STINGRAY_TOOLKIT_MEMBERLISTCOMPARER_H
#define STINGRAY_TOOLKIT_MEMBERLISTCOMPARER_H

#include <stingray/toolkit/MemberToValueComparer.h>
#include <stingray/toolkit/Tuple.h>
#include <stingray/toolkit/comparers.h>
#include <stingray/toolkit/shared_ptr.h>

namespace stingray
{

	template<typename MemberPointerT, typename ComparerT>
	struct CustomMemberComparerWrapper
	{
		typedef MemberExtractor<MemberPointerT> Extractor;
	private:
		MemberPointerT _memberPointer;
		ComparerT _comparer;
	public:
		CustomMemberComparerWrapper(MemberPointerT memberPointer, ComparerT comparer)
			: _memberPointer(memberPointer), _comparer(comparer)
		{}

		template <typename ClassType>
		int Compare(const ClassType &lhs, const ClassType &rhs) const
		{
			return _comparer(Extractor::GetValue(lhs, _memberPointer), Extractor::GetValue(rhs, _memberPointer));
		}
	};


	template<typename MemberPointerT, typename ComparerT>
	CustomMemberComparerWrapper<MemberPointerT, ComparerT> CustomMemberComparer(MemberPointerT pointer, ComparerT comparer)
	{ return CustomMemberComparerWrapper<MemberPointerT, ComparerT>(pointer, comparer); }


	template <typename MemberPointerTuple, typename DereferencingManager = AllowDereferencing>
	struct MemberListComparer
	{
	private:
		MemberPointerTuple _memberPointerList;
		template<typename MemberType>
		static int CompareMemberValue(MemberType lhs, MemberType rhs)
		{
			if (lhs < rhs)
				return -1;
			if (rhs < lhs)
				return 1;
			return 0;
		}
		template <typename ClassType, typename MemberPointerType>
		static int CompareMember(const ClassType &lhs, const ClassType &rhs, const MemberPointerType &pointer)
		{
			typedef MemberExtractor<MemberPointerType> Extractor;
			return CompareMemberValue(Extractor::GetValue(lhs, pointer), Extractor::GetValue(rhs, pointer));
		}
		template <typename ClassType, typename MemberPointerT, typename ComparerT>
		static int CompareMember(const ClassType &lhs, const ClassType &rhs, const CustomMemberComparerWrapper<MemberPointerT, ComparerT> &comparer)
		{
			return comparer.Compare(lhs, rhs);
		}
	public:
		template <typename ClassType>
		static int Compare(const ClassType &lhs, const ClassType &rhs, const MemberPointerTuple &tuple)
		{
			int result = CompareMember(lhs, rhs, tuple.GetHead());
			if (result == 0)
				return MemberListComparer<typename MemberPointerTuple::Tail>::Compare(lhs, rhs, tuple.GetTail());
			return result;
		}

		MemberListComparer(const MemberPointerTuple &memberPointerList)
			: _memberPointerList(memberPointerList)
		{}

		template <typename ClassType1, typename ClassType2>
		int operator ()(const ClassType1 &lhs, const ClassType2 &rhs) const
		{ return Compare(DereferencingManager::Process(lhs), DereferencingManager::Process(rhs), _memberPointerList); }
	};

	template < >
	struct MemberListComparer<Tuple<TypeListEndNode> >
	{
		template <typename ClassType>
		static int Compare(const ClassType &lhs, const ClassType &rhs, const Tuple<TypeListEndNode> &tuple)
		{ return 0; }
	};


	template <typename IntComparer, template<typename> class Adapter>
	struct CmpAdapter
	{
	private:
		IntComparer _comparer;
		Adapter<int> _adapter;
	public:
		CmpAdapter(): _comparer(), _adapter()
		{}
		CmpAdapter(const IntComparer& comparer, const Adapter<int> &adapter = Adapter<int>())
			: _comparer(comparer), _adapter(adapter)
		{}
		template <typename ClassType1, typename ClassType2>
		int operator ()(const ClassType1 &lhs, const ClassType2 &rhs) const
		{
			return _adapter(_comparer(lhs, rhs), 0);
		}
	};

#define TY typename
#define P_(N) const T##N& p##N
#define DETAIL_TOOLKIT_DECLARE_MAKEMEMBERLISTCOMPARER(N_, TypesDecl_, TypesUsage_, ParamsDecl_, ParamsUsage_) \
	template < TypesDecl_ > \
	MemberListComparer<Tuple<TypeList_##N_<TypesUsage_> > > CompareMemberListNonAdapted(ParamsDecl_) \
	{ return MemberListComparer<Tuple<TypeList_##N_<TypesUsage_> > >(Tuple<TypeList_##N_<TypesUsage_> >(ParamsUsage_)); } \
	template <template<typename> class Adapter, TypesDecl_> \
	CmpAdapter<MemberListComparer<Tuple<TypeList_##N_<TypesUsage_> > >, Adapter> CompareMemberList(ParamsDecl_, const Adapter<int> &adapter = Adapter<int>()) \
	{ return CmpAdapter<MemberListComparer<Tuple<TypeList_##N_<TypesUsage_> > >, Adapter>(Tuple<TypeList_##N_<TypesUsage_> >(ParamsUsage_), adapter); }


	DETAIL_TOOLKIT_DECLARE_MAKEMEMBERLISTCOMPARER(1, MK_PARAM(TY T1), MK_PARAM(T1), MK_PARAM(P_(1)), MK_PARAM(p1));
	DETAIL_TOOLKIT_DECLARE_MAKEMEMBERLISTCOMPARER(2, MK_PARAM(TY T1, TY T2), MK_PARAM(T1, T2), MK_PARAM(P_(1), P_(2)), MK_PARAM(p1, p2));
	DETAIL_TOOLKIT_DECLARE_MAKEMEMBERLISTCOMPARER(3, MK_PARAM(TY T1, TY T2, TY T3), MK_PARAM(T1, T2, T3), MK_PARAM(P_(1), P_(2), P_(3)), MK_PARAM(p1, p2, p3));
	DETAIL_TOOLKIT_DECLARE_MAKEMEMBERLISTCOMPARER(4, MK_PARAM(TY T1, TY T2, TY T3, TY T4), MK_PARAM(T1, T2, T3, T4), MK_PARAM(P_(1), P_(2), P_(3), P_(4)), MK_PARAM(p1, p2, p3, p4));
	DETAIL_TOOLKIT_DECLARE_MAKEMEMBERLISTCOMPARER(5, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5), MK_PARAM(T1, T2, T3, T4, T5), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5)), MK_PARAM(p1, p2, p3, p4, p5));
	DETAIL_TOOLKIT_DECLARE_MAKEMEMBERLISTCOMPARER(6, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6), MK_PARAM(T1, T2, T3, T4, T5, T6), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6)), MK_PARAM(p1, p2, p3, p4, p5, p6));
	DETAIL_TOOLKIT_DECLARE_MAKEMEMBERLISTCOMPARER(7, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7), MK_PARAM(T1, T2, T3, T4, T5, T6, T7), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7));
	DETAIL_TOOLKIT_DECLARE_MAKEMEMBERLISTCOMPARER(8, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8));
	DETAIL_TOOLKIT_DECLARE_MAKEMEMBERLISTCOMPARER(9, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9));
	DETAIL_TOOLKIT_DECLARE_MAKEMEMBERLISTCOMPARER(10, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10));
	DETAIL_TOOLKIT_DECLARE_MAKEMEMBERLISTCOMPARER(11, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11));
	DETAIL_TOOLKIT_DECLARE_MAKEMEMBERLISTCOMPARER(12, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12));
	DETAIL_TOOLKIT_DECLARE_MAKEMEMBERLISTCOMPARER(13, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13));
	DETAIL_TOOLKIT_DECLARE_MAKEMEMBERLISTCOMPARER(14, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13), P_(14)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14));
	DETAIL_TOOLKIT_DECLARE_MAKEMEMBERLISTCOMPARER(15, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13), P_(14), P_(15)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15));
	DETAIL_TOOLKIT_DECLARE_MAKEMEMBERLISTCOMPARER(16, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15, TY T16), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13), P_(14), P_(15), P_(16)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16));
	DETAIL_TOOLKIT_DECLARE_MAKEMEMBERLISTCOMPARER(17, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15, TY T16, TY T17), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13), P_(14), P_(15), P_(16), P_(17)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17));
	DETAIL_TOOLKIT_DECLARE_MAKEMEMBERLISTCOMPARER(18, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15, TY T16, TY T17, TY T18), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13), P_(14), P_(15), P_(16), P_(17), P_(18)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18));
	DETAIL_TOOLKIT_DECLARE_MAKEMEMBERLISTCOMPARER(19, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15, TY T16, TY T17, TY T18, TY T19), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13), P_(14), P_(15), P_(16), P_(17), P_(18), P_(19)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19));
	DETAIL_TOOLKIT_DECLARE_MAKEMEMBERLISTCOMPARER(20, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15, TY T16, TY T17, TY T18, TY T19, TY T20), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13), P_(14), P_(15), P_(16), P_(17), P_(18), P_(19), P_(20)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20));

#undef DETAIL_TOOLKIT_DECLARE_MAKEMEMBERLISTCOMPARER
#undef P_
#undef TY


	template <typename MemberPointerType, MemberPointerType MemberPointer, template <typename> class Comparer>
	struct PrimaryKeyHelper
	{
		typedef MemberExtractor<MemberPointerType> Extractor;
		typedef typename Extractor::ClassType ClassType;
		typedef typename Extractor::MemberType MemberType;
		typedef MemberToValueComparer<MemberPointerType, Comparer<MemberType> >		UnaryT;
		typedef MemberExtractorComparer<MemberPointerType, Comparer<MemberType>	>	BinaryToValueT;
		typedef MemberListComparer<Tuple<TypeList_1<MemberPointerType> > >			BinaryNonAdaptedT;
		typedef CmpAdapter<BinaryNonAdaptedT, Comparer>								BinaryT;

		static UnaryT Unary(const MemberType& value = MemberType())	{ return CompareMemberToValue<Comparer>(MemberPointer, value); }
		static BinaryToValueT BinaryToValue()						{ return CompareMember<Comparer>(MemberPointer); }
		static BinaryT Binary()										{ return CompareMemberList<Comparer>(MemberPointer); }
	};


}

#endif
