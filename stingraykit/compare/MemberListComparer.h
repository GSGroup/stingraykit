#ifndef STINGRAYKIT_COMPARE_MEMBERLISTCOMPARER_H
#define STINGRAYKIT_COMPARE_MEMBERLISTCOMPARER_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/compare/MemberToValueComparer.h>
#include <stingraykit/compare/comparers.h>
#include <stingraykit/Tuple.h>

namespace stingray
{

	namespace Detail
	{
		template < typename MemberPointerTuple >
		struct MemberListComparerImpl
		{
			template < typename ClassType, typename MemberPointerType >
			static int CompareMember(const ClassType& lhs, const ClassType& rhs, const MemberPointerType& pointer)
			{
				typedef MemberExtractor<MemberPointerType> Extractor;
				return comparers::Cmp()(Extractor::GetValue(lhs, pointer), Extractor::GetValue(rhs, pointer));
			}

			template < typename ClassType, typename MemberPointerT, typename ComparerT >
			static int CompareMember(const ClassType& lhs, const ClassType& rhs, const CustomMemberComparerWrapper<MemberPointerT, ComparerT>& comparer)
			{
				CompileTimeAssert<IsSame<typename function_info<ComparerT>::RetType, int>::Value> ErrorExpectedCmpComparer;
				(void)ErrorExpectedCmpComparer;

				return comparer.Compare(lhs, rhs);
			}

			template < typename ClassType >
			static int Do(const ClassType& lhs, const ClassType& rhs, const MemberPointerTuple& tuple)
			{
				const int result = CompareMember(lhs, rhs, tuple.GetHead());
				if (result != 0)
					return result;

				return MemberListComparerImpl<typename MemberPointerTuple::Tail>::Do(lhs, rhs, tuple.GetTail());
			}
		};


		template < >
		struct MemberListComparerImpl<Tuple<TypeListEndNode> >
		{
			template < typename ClassType >
			static int Do(const ClassType& lhs, const ClassType& rhs, const Tuple<TypeListEndNode>& tuple)
			{ return 0; }
		};
	}


	template < typename MemberPointerTuple >
	struct MemberListComparer : public comparers::CmpComparerBase<MemberListComparer<MemberPointerTuple> >
	{
	private:
		MemberPointerTuple _memberPointerList;

	public:
		MemberListComparer(const MemberPointerTuple& memberPointerList) : _memberPointerList(memberPointerList)
		{ }

		template < typename T >
		int DoCompare(const T& lhs, const T& rhs) const
		{ return Detail::MemberListComparerImpl<MemberPointerTuple>::Do(lhs, rhs, _memberPointerList); }
	};


	template < typename... Ts >
	MemberListComparer<Tuple<typename TypeList<Ts...>::type> > CompareMembersCmp(const Ts&... args)
	{ return MemberListComparer<Tuple<typename TypeList<Ts...>::type> >(Tuple<typename TypeList<Ts...>::type>(args...)); }

	template < typename... Ts >
	comparers::CmpToLess<MemberListComparer<Tuple<typename TypeList<Ts...>::type> > > CompareMembersLess(const Ts&... args)
	{ return comparers::CmpToLess<MemberListComparer<Tuple<typename TypeList<Ts...>::type> > >(Tuple<typename TypeList<Ts...>::type>(args...)); }

	template < typename... Ts >
	comparers::CmpToGreater<MemberListComparer<Tuple<typename TypeList<Ts...>::type> > > CompareMembersGreater(const Ts&... args)
	{ return comparers::CmpToGreater<MemberListComparer<Tuple<typename TypeList<Ts...>::type> > >(Tuple<typename TypeList<Ts...>::type>(args...)); }

	template < typename... Ts >
	comparers::CmpToEquals<MemberListComparer<Tuple<typename TypeList<Ts...>::type> > > CompareMembersEquals(const Ts&... args)
	{ return comparers::CmpToEquals<MemberListComparer<Tuple<typename TypeList<Ts...>::type> > >(Tuple<typename TypeList<Ts...>::type>(args...)); }

}

#endif
