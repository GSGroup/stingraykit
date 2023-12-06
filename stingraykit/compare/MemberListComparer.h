#ifndef STINGRAYKIT_COMPARE_MEMBERLISTCOMPARER_H
#define STINGRAYKIT_COMPARE_MEMBERLISTCOMPARER_H

// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
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
			template < typename ClassType, typename MemberPointer >
			static int CompareMember(const ClassType& lhs, const ClassType& rhs, const MemberPointer& ptr)
			{
				using Extractor = MemberExtractor<MemberPointer>;
				return comparers::Cmp()(Extractor::GetValue(lhs, ptr), Extractor::GetValue(rhs, ptr));
			}

			template < typename ClassType, typename MemberPointer, typename Comparer >
			static int CompareMember(const ClassType& lhs, const ClassType& rhs, const CustomMemberComparerWrapper<MemberPointer, Comparer>& comparer)
			{
				static_assert(IsSame<typename function_info<Comparer>::RetType, int>::Value, "Expected Cmp comparer");
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
		struct MemberListComparerImpl<Tuple<TypeListEndNode>>
		{
			template < typename ClassType >
			static int Do(const ClassType& lhs, const ClassType& rhs, const Tuple<TypeListEndNode>& tuple)
			{ return 0; }
		};
	}


	template < typename MemberPointerTuple >
	struct MemberListComparer : public comparers::CmpComparerBase<MemberListComparer<MemberPointerTuple>>
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
	MemberListComparer<Tuple<TypeList<Ts...>>> CompareMembersCmp(const Ts&... args)
	{ return MemberListComparer<Tuple<TypeList<Ts...>>>(Tuple<TypeList<Ts...>>(args...)); }

	template < typename... Ts >
	comparers::CmpToLess<MemberListComparer<Tuple<TypeList<Ts...>>>> CompareMembersLess(const Ts&... args)
	{ return comparers::CmpToLess<MemberListComparer<Tuple<TypeList<Ts...>>>>(Tuple<TypeList<Ts...>>(args...)); }

	template < typename... Ts >
	comparers::CmpToGreater<MemberListComparer<Tuple<TypeList<Ts...>>>> CompareMembersGreater(const Ts&... args)
	{ return comparers::CmpToGreater<MemberListComparer<Tuple<TypeList<Ts...>>>>(Tuple<TypeList<Ts...>>(args...)); }

	template < typename... Ts >
	comparers::CmpToEquals<MemberListComparer<Tuple<TypeList<Ts...>>>> CompareMembersEquals(const Ts&... args)
	{ return comparers::CmpToEquals<MemberListComparer<Tuple<TypeList<Ts...>>>>(Tuple<TypeList<Ts...>>(args...)); }

}

#endif
