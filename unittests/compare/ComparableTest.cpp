#include <stingraykit/compare/Comparable.h>
#include <stingraykit/compare/comparers.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	struct ComparableFirst final : public Comparable<ComparableFirst>
	{
		const int			Data;

		mutable unsigned	CompareCalls = 0;

		explicit ComparableFirst(int data) : Data(data) { }

	private:
		int DoCompare(const ComparableFirst& other) const
		{
			++CompareCalls;
			return comparers::Cmp()(Data, other.Data);
		}
	};

	struct ComparableSecond final : public Comparable<ComparableSecond>
	{
		const int			Data;

		mutable unsigned	CompareCalls = 0;

		explicit ComparableSecond(int data) : Data(data) { }

	private:
		int DoCompare(const ComparableSecond& other) const
		{
			++CompareCalls;
			return comparers::Cmp()(Data, other.Data);
		}
	};

}


TEST(ComparableTest, SameType)
{
	{
		ComparableFirst o1(1);
		ComparableFirst o2(1);

		ASSERT_EQ(comparers::Cmp()(o1, o2), 0);
		ASSERT_FALSE(comparers::Less()(o1, o2));
		ASSERT_TRUE(comparers::Equals()(o1, o2));
		ASSERT_FALSE(comparers::Greater()(o1, o2));

		ASSERT_EQ(o1.CompareCalls, 4u);
		ASSERT_EQ(o2.CompareCalls, 0u);
	}

	{
		ComparableFirst o1(1);
		ComparableFirst o2(2);

		ASSERT_LT(comparers::Cmp()(o1, o2), 0);
		ASSERT_TRUE(comparers::Less()(o1, o2));
		ASSERT_FALSE(comparers::Equals()(o1, o2));
		ASSERT_FALSE(comparers::Greater()(o1, o2));

		ASSERT_EQ(o1.CompareCalls, 4u);
		ASSERT_EQ(o2.CompareCalls, 0u);
	}

	{
		ComparableFirst o1(2);
		ComparableFirst o2(1);

		ASSERT_GT(comparers::Cmp()(o1, o2), 0);
		ASSERT_FALSE(comparers::Less()(o1, o2));
		ASSERT_FALSE(comparers::Equals()(o1, o2));
		ASSERT_TRUE(comparers::Greater()(o1, o2));

		ASSERT_EQ(o1.CompareCalls, 4u);
		ASSERT_EQ(o2.CompareCalls, 0u);
	}
}


TEST(ComparableTest, DifferentType)
{
	{
		ComparableFirst o1(1);
		ComparableSecond o2(1);

		ASSERT_NE(comparers::Cmp()(o1, o2), 0);
		ASSERT_NE(comparers::Less()(o1, o2), comparers::Less()(o2, o1));
		ASSERT_FALSE(comparers::Equals()(o1, o2));
		ASSERT_NE(comparers::Greater()(o1, o2), comparers::Greater()(o2, o1));

		ASSERT_EQ(o1.CompareCalls, 0u);
		ASSERT_EQ(o2.CompareCalls, 0u);
	}

	{
		ComparableFirst o1(1);
		ComparableSecond o2(2);

		ASSERT_NE(comparers::Cmp()(o1, o2), 0);
		ASSERT_NE(comparers::Less()(o1, o2), comparers::Less()(o2, o1));
		ASSERT_FALSE(comparers::Equals()(o1, o2));
		ASSERT_NE(comparers::Greater()(o1, o2), comparers::Greater()(o2, o1));

		ASSERT_EQ(o1.CompareCalls, 0u);
		ASSERT_EQ(o2.CompareCalls, 0u);
	}

	{
		ComparableFirst o1(2);
		ComparableSecond o2(1);

		ASSERT_NE(comparers::Cmp()(o1, o2), 0);
		ASSERT_NE(comparers::Less()(o1, o2), comparers::Less()(o2, o1));
		ASSERT_FALSE(comparers::Equals()(o1, o2));
		ASSERT_NE(comparers::Greater()(o1, o2), comparers::Greater()(o2, o1));

		ASSERT_EQ(o1.CompareCalls, 0u);
		ASSERT_EQ(o2.CompareCalls, 0u);
	}
}
