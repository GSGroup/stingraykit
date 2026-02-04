// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/compare/comparers.h>
#include <stingraykit/compare/CustomComparable.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	struct ComparableByXTag { };
	struct ComparableByYTag { };

	struct ComparableFirst final : public CustomComparable<ComparableByXTag, ComparableFirst>, public CustomComparable<ComparableByYTag, ComparableFirst>
	{
		const int			X;
		const int			Y;

		ComparableFirst(int x, int y) : X(x), Y(y) { }

	private:
		int DoCustomCompare(ComparableByXTag, const ComparableFirst& other) const
		{ return comparers::Cmp()(X, other.X); }

		int DoCustomCompare(ComparableByYTag, const ComparableFirst& other) const
		{ return comparers::Cmp()(Y, other.Y); }
	};

	struct ComparableSecond final : public CustomComparable<ComparableByXTag, ComparableSecond>, public CustomComparable<ComparableByYTag, ComparableSecond>
	{
		const int			X;
		const int			Y;

		ComparableSecond(int x, int y) : X(x), Y(y) { }

	private:
		int DoCustomCompare(ComparableByXTag, const ComparableSecond& other) const
		{ return comparers::Cmp()(X, other.X); }

		int DoCustomCompare(ComparableByYTag, const ComparableSecond& other) const
		{ return comparers::Cmp()(Y, other.Y); }
	};

	struct ByXCmp : public comparers::CmpComparerBase<ByXCmp>
	{
		int DoCompare(const ICustomComparable<ComparableByXTag>& lhs, const ICustomComparable<ComparableByXTag>& rhs) const
		{ return lhs.CustomCompare(rhs); }
	};
	STINGRAYKIT_DECLARE_COMPARERS(ByX);

	struct ByYCmp : public comparers::CmpComparerBase<ByYCmp>
	{
		int DoCompare(const ICustomComparable<ComparableByYTag>& lhs, const ICustomComparable<ComparableByYTag>& rhs) const
		{ return lhs.CustomCompare(rhs); }
	};
	STINGRAYKIT_DECLARE_COMPARERS(ByY);

}


TEST(CustomComparableTest, SameType)
{
	{
		ComparableFirst o1(1, 2);
		ComparableFirst o2(1, 2);

		ASSERT_EQ(ByXCmp()(o1, o2), 0);
		ASSERT_FALSE(ByXLess()(o1, o2));
		ASSERT_TRUE(ByXEquals()(o1, o2));
		ASSERT_FALSE(ByXGreater()(o1, o2));

		ASSERT_EQ(ByYCmp()(o1, o2), 0);
		ASSERT_FALSE(ByYLess()(o1, o2));
		ASSERT_TRUE(ByYEquals()(o1, o2));
		ASSERT_FALSE(ByYGreater()(o1, o2));
	}

	{
		ComparableFirst o1(1, 2);
		ComparableFirst o2(2, 1);

		ASSERT_LT(ByXCmp()(o1, o2), 0);
		ASSERT_TRUE(ByXLess()(o1, o2));
		ASSERT_FALSE(ByXEquals()(o1, o2));
		ASSERT_FALSE(ByXGreater()(o1, o2));

		ASSERT_GT(ByYCmp()(o1, o2), 0);
		ASSERT_FALSE(ByYLess()(o1, o2));
		ASSERT_FALSE(ByYEquals()(o1, o2));
		ASSERT_TRUE(ByYGreater()(o1, o2));
	}

	{
		ComparableFirst o1(2, 1);
		ComparableFirst o2(1, 2);

		ASSERT_GT(ByXCmp()(o1, o2), 0);
		ASSERT_FALSE(ByXLess()(o1, o2));
		ASSERT_FALSE(ByXEquals()(o1, o2));
		ASSERT_TRUE(ByXGreater()(o1, o2));

		ASSERT_LT(ByYCmp()(o1, o2), 0);
		ASSERT_TRUE(ByYLess()(o1, o2));
		ASSERT_FALSE(ByYEquals()(o1, o2));
		ASSERT_FALSE(ByYGreater()(o1, o2));
	}
}


TEST(CustomComparableTest, DifferentType)
{
	{
		ComparableFirst o1(1, 2);
		ComparableSecond o2(1, 2);

		ASSERT_NE(ByXCmp()(o1, o2), 0);
		ASSERT_NE(ByXLess()(o1, o2), ByXLess()(o2, o1));
		ASSERT_FALSE(ByXEquals()(o1, o2));
		ASSERT_NE(ByXGreater()(o1, o2), ByXGreater()(o2, o1));

		ASSERT_NE(ByYCmp()(o1, o2), 0);
		ASSERT_NE(ByYLess()(o1, o2), ByYLess()(o2, o1));
		ASSERT_FALSE(ByYEquals()(o1, o2));
		ASSERT_NE(ByYGreater()(o1, o2), ByYGreater()(o2, o1));
	}

	{
		ComparableFirst o1(1, 2);
		ComparableSecond o2(2, 1);

		ASSERT_NE(ByXCmp()(o1, o2), 0);
		ASSERT_NE(ByXLess()(o1, o2), ByXLess()(o2, o1));
		ASSERT_FALSE(ByXEquals()(o1, o2));
		ASSERT_NE(ByXGreater()(o1, o2), ByXGreater()(o2, o1));

		ASSERT_NE(ByYCmp()(o1, o2), 0);
		ASSERT_NE(ByYLess()(o1, o2), ByYLess()(o2, o1));
		ASSERT_FALSE(ByYEquals()(o1, o2));
		ASSERT_NE(ByYGreater()(o1, o2), ByYGreater()(o2, o1));
	}

	{
		ComparableFirst o1(2, 1);
		ComparableSecond o2(1, 2);

		ASSERT_NE(ByXCmp()(o1, o2), 0);
		ASSERT_NE(ByXLess()(o1, o2), ByXLess()(o2, o1));
		ASSERT_FALSE(ByXEquals()(o1, o2));
		ASSERT_NE(ByXGreater()(o1, o2), ByXGreater()(o2, o1));

		ASSERT_NE(ByYCmp()(o1, o2), 0);
		ASSERT_NE(ByYLess()(o1, o2), ByYLess()(o2, o1));
		ASSERT_FALSE(ByYEquals()(o1, o2));
		ASSERT_NE(ByYGreater()(o1, o2), ByYGreater()(o2, o1));
	}
}
