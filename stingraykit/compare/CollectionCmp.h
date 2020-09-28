#ifndef STINGRAYKIT_COMPARE_COLLECTIONCMP_H
#define STINGRAYKIT_COMPARE_COLLECTIONCMP_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/Range.h>
#include <stingraykit/compare/comparers.h>

namespace stingray
{

	template < typename ItemCmp = comparers::Cmp >
	struct CollectionCmp : public comparers::CmpComparerBase<CollectionCmp<ItemCmp> >
	{
		template < typename T >
		int DoCompare(const T& lhs, const T& rhs) const
		{
			static_assert(IsSame<typename function_info<ItemCmp>::RetType, int>::Value, "Expected Cmp comparer");
			return DoCompareRanges(ToRange(lhs), ToRange(rhs));
		}

	private:
		template < typename Range_ >
		int DoCompareRanges(Range_ lhs, Range_ rhs) const
		{
			for (; lhs; ++lhs, ++rhs)
			{
				if (!rhs)
					return 1;

				const int itemResult = ItemCmp()(*lhs, *rhs);
				if (itemResult != 0)
					return itemResult;
			}
			return rhs ? -1 : 0;
		}
	};

}

#endif
