#ifndef STINGRAYKIT_COMPARE_COLLECTIONCMP_H
#define STINGRAYKIT_COMPARE_COLLECTIONCMP_H

#include <stingraykit/compare/comparers.h>
#include <stingraykit/collection/Range.h>

namespace stingray
{

	template<typename ItemCmp = comparers::Cmp>
	struct CollectionCmp : public comparers::CmpComparerBase<CollectionCmp<ItemCmp> >
	{
		template < typename T >
		int DoCompare(const T& lhs, const T& rhs) const
		{
			CompileTimeAssert<SameType<typename function_info<ItemCmp>::RetType, int>::Value> ErrorExpectedCmpComparer;
			(void)ErrorExpectedCmpComparer;
			return DoCompareRanges(ToRange(lhs), ToRange(rhs));
		}

	private:
		template <typename Range_>
		int DoCompareRanges(Range_ lhs, Range_ rhs) const
		{
			for (; lhs; ++lhs, ++rhs)
			{
				if (!rhs)
					return -1;

				int item_result = ItemCmp()(*lhs, *rhs);
				if (item_result != 0)
					return item_result;
			}
			return rhs ? 1 : 0;
		}
	};

}

#endif
