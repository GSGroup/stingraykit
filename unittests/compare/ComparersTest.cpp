// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/compare/IFuzzyEquatable.h>
#include <stingraykit/compare/comparers.h>
#include <stingraykit/math.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	struct ObjectWithCompare final
	{
		const int			Data;

		mutable bool		CompareWasCalled = false;

		explicit ObjectWithCompare(int data) : Data(data) { }

		int Compare(const ObjectWithCompare& other) const
		{
			CompareWasCalled = true;
			return comparers::Cmp()(Data, other.Data);
		}
	};

	struct ObjectWithLessAndEquals final
	{
		const int			Data;

		mutable bool		LessWasCalled = false;
		mutable bool		EqualsWasCalled = false;

		explicit ObjectWithLessAndEquals(int data) : Data(data) { }

		bool operator < (const ObjectWithLessAndEquals& other) const
		{
			LessWasCalled = true;
			return Data < other.Data;
		}

		bool operator == (const ObjectWithLessAndEquals& other) const
		{
			EqualsWasCalled = true;
			return Data == other.Data;
		}
	};

	struct ObjectWithCompareAndLessAndEquals final
	{
		const int			Data;

		mutable bool		CompareWasCalled = false;
		mutable bool		LessWasCalled = false;
		mutable bool		EqualsWasCalled = false;

		explicit ObjectWithCompareAndLessAndEquals(int data) : Data(data) { }

		int Compare(const ObjectWithCompareAndLessAndEquals& other) const
		{
			CompareWasCalled = true;
			return comparers::Cmp()(Data, other.Data);
		}

		bool operator < (const ObjectWithCompareAndLessAndEquals& other) const
		{
			LessWasCalled = true;
			return Data < other.Data;
		}

		bool operator == (const ObjectWithCompareAndLessAndEquals& other) const
		{
			EqualsWasCalled = true;
			return Data == other.Data;
		}
	};

}


TEST(ComparersTest, Compare)
{
	{
		ObjectWithCompare o1(1);
		ObjectWithCompare o2(1);

		ASSERT_EQ(comparers::Cmp()(o1, o2), 0);
		ASSERT_TRUE(o1.CompareWasCalled);
		ASSERT_FALSE(o2.CompareWasCalled);
	}

	{
		ObjectWithCompare o1(1);
		ObjectWithCompare o2(2);

		ASSERT_LT(comparers::Cmp()(o1, o2), 0);
		ASSERT_TRUE(o1.CompareWasCalled);
		ASSERT_FALSE(o2.CompareWasCalled);
	}

	{
		ObjectWithCompare o1(2);
		ObjectWithCompare o2(1);

		ASSERT_GT(comparers::Cmp()(o1, o2), 0);
		ASSERT_TRUE(o1.CompareWasCalled);
		ASSERT_FALSE(o2.CompareWasCalled);
	}

	{
		ObjectWithCompare o1(1);
		ObjectWithCompare o2(1);

		ASSERT_FALSE(comparers::Less()(o1, o2));
		ASSERT_TRUE(o1.CompareWasCalled);
		ASSERT_FALSE(o2.CompareWasCalled);
	}

	{
		ObjectWithCompare o1(1);
		ObjectWithCompare o2(2);

		ASSERT_TRUE(comparers::Less()(o1, o2));
		ASSERT_TRUE(o1.CompareWasCalled);
		ASSERT_FALSE(o2.CompareWasCalled);
	}

	{
		ObjectWithCompare o1(2);
		ObjectWithCompare o2(1);

		ASSERT_FALSE(comparers::Less()(o1, o2));
		ASSERT_TRUE(o1.CompareWasCalled);
		ASSERT_FALSE(o2.CompareWasCalled);
	}

	{
		ObjectWithCompare o1(1);
		ObjectWithCompare o2(1);

		ASSERT_TRUE(comparers::Equals()(o1, o2));
		ASSERT_TRUE(o1.CompareWasCalled);
		ASSERT_FALSE(o2.CompareWasCalled);
	}

	{
		ObjectWithCompare o1(1);
		ObjectWithCompare o2(2);

		ASSERT_FALSE(comparers::Equals()(o1, o2));
		ASSERT_TRUE(o1.CompareWasCalled);
		ASSERT_FALSE(o2.CompareWasCalled);
	}

	{
		ObjectWithCompare o1(2);
		ObjectWithCompare o2(1);

		ASSERT_FALSE(comparers::Equals()(o1, o2));
		ASSERT_TRUE(o1.CompareWasCalled);
		ASSERT_FALSE(o2.CompareWasCalled);
	}

	{
		ObjectWithCompare o1(1);
		ObjectWithCompare o2(1);

		ASSERT_FALSE(comparers::Greater()(o1, o2));
		ASSERT_TRUE(o1.CompareWasCalled);
		ASSERT_FALSE(o2.CompareWasCalled);
	}

	{
		ObjectWithCompare o1(1);
		ObjectWithCompare o2(2);

		ASSERT_FALSE(comparers::Greater()(o1, o2));
		ASSERT_TRUE(o1.CompareWasCalled);
		ASSERT_FALSE(o2.CompareWasCalled);
	}

	{
		ObjectWithCompare o1(2);
		ObjectWithCompare o2(1);

		ASSERT_TRUE(comparers::Greater()(o1, o2));
		ASSERT_TRUE(o1.CompareWasCalled);
		ASSERT_FALSE(o2.CompareWasCalled);
	}
}


TEST(ComparersTest, LessAndEquals)
{
	{
		ObjectWithLessAndEquals o1(1);
		ObjectWithLessAndEquals o2(1);

		ASSERT_EQ(comparers::Cmp()(o1, o2), 0);
		ASSERT_TRUE(o1.LessWasCalled);
		ASSERT_FALSE(o1.EqualsWasCalled);
		ASSERT_TRUE(o2.LessWasCalled);
		ASSERT_FALSE(o2.EqualsWasCalled);
	}

	{
		ObjectWithLessAndEquals o1(1);
		ObjectWithLessAndEquals o2(2);

		ASSERT_LT(comparers::Cmp()(o1, o2), 0);
		ASSERT_TRUE(o1.LessWasCalled);
		ASSERT_FALSE(o1.EqualsWasCalled);
		ASSERT_FALSE(o2.LessWasCalled);
		ASSERT_FALSE(o2.EqualsWasCalled);
	}

	{
		ObjectWithLessAndEquals o1(2);
		ObjectWithLessAndEquals o2(1);

		ASSERT_GT(comparers::Cmp()(o1, o2), 0);
		ASSERT_TRUE(o1.LessWasCalled);
		ASSERT_FALSE(o1.EqualsWasCalled);
		ASSERT_TRUE(o2.LessWasCalled);
		ASSERT_FALSE(o2.EqualsWasCalled);
	}

	{
		ObjectWithLessAndEquals o1(1);
		ObjectWithLessAndEquals o2(1);

		ASSERT_FALSE(comparers::Less()(o1, o2));
		ASSERT_TRUE(o1.LessWasCalled);
		ASSERT_FALSE(o1.EqualsWasCalled);
		ASSERT_FALSE(o2.LessWasCalled);
		ASSERT_FALSE(o2.EqualsWasCalled);
	}

	{
		ObjectWithLessAndEquals o1(1);
		ObjectWithLessAndEquals o2(2);

		ASSERT_TRUE(comparers::Less()(o1, o2));
		ASSERT_TRUE(o1.LessWasCalled);
		ASSERT_FALSE(o1.EqualsWasCalled);
		ASSERT_FALSE(o2.LessWasCalled);
		ASSERT_FALSE(o2.EqualsWasCalled);
	}

	{
		ObjectWithLessAndEquals o1(2);
		ObjectWithLessAndEquals o2(1);

		ASSERT_FALSE(comparers::Less()(o1, o2));
		ASSERT_TRUE(o1.LessWasCalled);
		ASSERT_FALSE(o1.EqualsWasCalled);
		ASSERT_FALSE(o2.LessWasCalled);
		ASSERT_FALSE(o2.EqualsWasCalled);
	}

	{
		ObjectWithLessAndEquals o1(1);
		ObjectWithLessAndEquals o2(1);

		ASSERT_TRUE(comparers::Equals()(o1, o2));
		ASSERT_FALSE(o1.LessWasCalled);
		ASSERT_TRUE(o1.EqualsWasCalled);
		ASSERT_FALSE(o2.LessWasCalled);
		ASSERT_FALSE(o2.EqualsWasCalled);
	}

	{
		ObjectWithLessAndEquals o1(1);
		ObjectWithLessAndEquals o2(2);

		ASSERT_FALSE(comparers::Equals()(o1, o2));
		ASSERT_FALSE(o1.LessWasCalled);
		ASSERT_TRUE(o1.EqualsWasCalled);
		ASSERT_FALSE(o2.LessWasCalled);
		ASSERT_FALSE(o2.EqualsWasCalled);
	}

	{
		ObjectWithLessAndEquals o1(2);
		ObjectWithLessAndEquals o2(1);

		ASSERT_FALSE(comparers::Equals()(o1, o2));
		ASSERT_FALSE(o1.LessWasCalled);
		ASSERT_TRUE(o1.EqualsWasCalled);
		ASSERT_FALSE(o2.LessWasCalled);
		ASSERT_FALSE(o2.EqualsWasCalled);
	}

	{
		ObjectWithLessAndEquals o1(1);
		ObjectWithLessAndEquals o2(1);

		ASSERT_FALSE(comparers::Greater()(o1, o2));
		ASSERT_FALSE(o1.LessWasCalled);
		ASSERT_FALSE(o1.EqualsWasCalled);
		ASSERT_TRUE(o2.LessWasCalled);
		ASSERT_FALSE(o2.EqualsWasCalled);
	}

	{
		ObjectWithLessAndEquals o1(1);
		ObjectWithLessAndEquals o2(2);

		ASSERT_FALSE(comparers::Greater()(o1, o2));
		ASSERT_FALSE(o1.LessWasCalled);
		ASSERT_FALSE(o1.EqualsWasCalled);
		ASSERT_TRUE(o2.LessWasCalled);
		ASSERT_FALSE(o2.EqualsWasCalled);
	}

	{
		ObjectWithLessAndEquals o1(2);
		ObjectWithLessAndEquals o2(1);

		ASSERT_TRUE(comparers::Greater()(o1, o2));
		ASSERT_FALSE(o1.LessWasCalled);
		ASSERT_FALSE(o1.EqualsWasCalled);
		ASSERT_TRUE(o2.LessWasCalled);
		ASSERT_FALSE(o2.EqualsWasCalled);
	}
}


TEST(ComparersTest, CompareAndLessAndEquals)
{
	{
		ObjectWithCompareAndLessAndEquals o1(1);
		ObjectWithCompareAndLessAndEquals o2(1);

		ASSERT_EQ(comparers::Cmp()(o1, o2), 0);
		ASSERT_TRUE(o1.CompareWasCalled);
		ASSERT_FALSE(o1.LessWasCalled);
		ASSERT_FALSE(o1.EqualsWasCalled);
		ASSERT_FALSE(o2.CompareWasCalled);
		ASSERT_FALSE(o2.LessWasCalled);
		ASSERT_FALSE(o2.EqualsWasCalled);
	}

	{
		ObjectWithCompareAndLessAndEquals o1(1);
		ObjectWithCompareAndLessAndEquals o2(2);

		ASSERT_LT(comparers::Cmp()(o1, o2), 0);
		ASSERT_TRUE(o1.CompareWasCalled);
		ASSERT_FALSE(o1.LessWasCalled);
		ASSERT_FALSE(o1.EqualsWasCalled);
		ASSERT_FALSE(o2.CompareWasCalled);
		ASSERT_FALSE(o2.LessWasCalled);
		ASSERT_FALSE(o2.EqualsWasCalled);
	}

	{
		ObjectWithCompareAndLessAndEquals o1(2);
		ObjectWithCompareAndLessAndEquals o2(1);

		ASSERT_GT(comparers::Cmp()(o1, o2), 0);
		ASSERT_TRUE(o1.CompareWasCalled);
		ASSERT_FALSE(o1.LessWasCalled);
		ASSERT_FALSE(o1.EqualsWasCalled);
		ASSERT_FALSE(o2.CompareWasCalled);
		ASSERT_FALSE(o2.LessWasCalled);
		ASSERT_FALSE(o2.EqualsWasCalled);
	}

	{
		ObjectWithCompareAndLessAndEquals o1(1);
		ObjectWithCompareAndLessAndEquals o2(1);

		ASSERT_FALSE(comparers::Less()(o1, o2));
		ASSERT_TRUE(o1.CompareWasCalled);
		ASSERT_FALSE(o1.LessWasCalled);
		ASSERT_FALSE(o1.EqualsWasCalled);
		ASSERT_FALSE(o2.CompareWasCalled);
		ASSERT_FALSE(o2.LessWasCalled);
		ASSERT_FALSE(o2.EqualsWasCalled);
	}

	{
		ObjectWithCompareAndLessAndEquals o1(1);
		ObjectWithCompareAndLessAndEquals o2(2);

		ASSERT_TRUE(comparers::Less()(o1, o2));
		ASSERT_TRUE(o1.CompareWasCalled);
		ASSERT_FALSE(o1.LessWasCalled);
		ASSERT_FALSE(o1.EqualsWasCalled);
		ASSERT_FALSE(o2.CompareWasCalled);
		ASSERT_FALSE(o2.LessWasCalled);
		ASSERT_FALSE(o2.EqualsWasCalled);
	}

	{
		ObjectWithCompareAndLessAndEquals o1(2);
		ObjectWithCompareAndLessAndEquals o2(1);

		ASSERT_FALSE(comparers::Less()(o1, o2));
		ASSERT_TRUE(o1.CompareWasCalled);
		ASSERT_FALSE(o1.LessWasCalled);
		ASSERT_FALSE(o1.EqualsWasCalled);
		ASSERT_FALSE(o2.CompareWasCalled);
		ASSERT_FALSE(o2.LessWasCalled);
		ASSERT_FALSE(o2.EqualsWasCalled);
	}

	{
		ObjectWithCompareAndLessAndEquals o1(1);
		ObjectWithCompareAndLessAndEquals o2(1);

		ASSERT_TRUE(comparers::Equals()(o1, o2));
		ASSERT_TRUE(o1.CompareWasCalled);
		ASSERT_FALSE(o1.LessWasCalled);
		ASSERT_FALSE(o1.EqualsWasCalled);
		ASSERT_FALSE(o2.CompareWasCalled);
		ASSERT_FALSE(o2.LessWasCalled);
		ASSERT_FALSE(o2.EqualsWasCalled);
	}

	{
		ObjectWithCompareAndLessAndEquals o1(1);
		ObjectWithCompareAndLessAndEquals o2(2);

		ASSERT_FALSE(comparers::Equals()(o1, o2));
		ASSERT_TRUE(o1.CompareWasCalled);
		ASSERT_FALSE(o1.LessWasCalled);
		ASSERT_FALSE(o1.EqualsWasCalled);
		ASSERT_FALSE(o2.CompareWasCalled);
		ASSERT_FALSE(o2.LessWasCalled);
		ASSERT_FALSE(o2.EqualsWasCalled);
	}

	{
		ObjectWithCompareAndLessAndEquals o1(2);
		ObjectWithCompareAndLessAndEquals o2(1);

		ASSERT_FALSE(comparers::Equals()(o1, o2));
		ASSERT_TRUE(o1.CompareWasCalled);
		ASSERT_FALSE(o1.LessWasCalled);
		ASSERT_FALSE(o1.EqualsWasCalled);
		ASSERT_FALSE(o2.CompareWasCalled);
		ASSERT_FALSE(o2.LessWasCalled);
		ASSERT_FALSE(o2.EqualsWasCalled);
	}

	{
		ObjectWithCompareAndLessAndEquals o1(1);
		ObjectWithCompareAndLessAndEquals o2(1);

		ASSERT_FALSE(comparers::Greater()(o1, o2));
		ASSERT_TRUE(o1.CompareWasCalled);
		ASSERT_FALSE(o1.LessWasCalled);
		ASSERT_FALSE(o1.EqualsWasCalled);
		ASSERT_FALSE(o2.CompareWasCalled);
		ASSERT_FALSE(o2.LessWasCalled);
		ASSERT_FALSE(o2.EqualsWasCalled);
	}

	{
		ObjectWithCompareAndLessAndEquals o1(1);
		ObjectWithCompareAndLessAndEquals o2(2);

		ASSERT_FALSE(comparers::Greater()(o1, o2));
		ASSERT_TRUE(o1.CompareWasCalled);
		ASSERT_FALSE(o1.LessWasCalled);
		ASSERT_FALSE(o1.EqualsWasCalled);
		ASSERT_FALSE(o2.CompareWasCalled);
		ASSERT_FALSE(o2.LessWasCalled);
		ASSERT_FALSE(o2.EqualsWasCalled);
	}

	{
		ObjectWithCompareAndLessAndEquals o1(2);
		ObjectWithCompareAndLessAndEquals o2(1);

		ASSERT_TRUE(comparers::Greater()(o1, o2));
		ASSERT_TRUE(o1.CompareWasCalled);
		ASSERT_FALSE(o1.LessWasCalled);
		ASSERT_FALSE(o1.EqualsWasCalled);
		ASSERT_FALSE(o2.CompareWasCalled);
		ASSERT_FALSE(o2.LessWasCalled);
		ASSERT_FALSE(o2.EqualsWasCalled);
	}
}


namespace
{

	struct ObjectWithFuzzyEquals final
	{
		const int			Data;

		mutable bool		FuzzyEqualsWasCalled = false;

		explicit ObjectWithFuzzyEquals(int data) : Data(data) { }

		bool FuzzyEquals(const ObjectWithFuzzyEquals& other) const
		{
			FuzzyEqualsWasCalled = true;
			return Abs(Data - other.Data) <= 1;
		}
	};

}


TEST(ComparersTest, FuzzyEquals)
{
	{
		ObjectWithFuzzyEquals o1(1);
		ObjectWithFuzzyEquals o2(1);

		ASSERT_TRUE(FuzzyEquals()(o1, o2));
		ASSERT_TRUE(o1.FuzzyEqualsWasCalled);
		ASSERT_FALSE(o2.FuzzyEqualsWasCalled);
	}

	{
		ObjectWithFuzzyEquals o1(1);
		ObjectWithFuzzyEquals o2(2);

		ASSERT_TRUE(FuzzyEquals()(o1, o2));
		ASSERT_TRUE(o1.FuzzyEqualsWasCalled);
		ASSERT_FALSE(o2.FuzzyEqualsWasCalled);
	}

	{
		ObjectWithFuzzyEquals o1(1);
		ObjectWithFuzzyEquals o2(3);

		ASSERT_FALSE(FuzzyEquals()(o1, o2));
		ASSERT_TRUE(o1.FuzzyEqualsWasCalled);
		ASSERT_FALSE(o2.FuzzyEqualsWasCalled);
	}

	{
		ObjectWithFuzzyEquals o1(3);
		ObjectWithFuzzyEquals o2(1);

		ASSERT_FALSE(FuzzyEquals()(o1, o2));
		ASSERT_TRUE(o1.FuzzyEqualsWasCalled);
		ASSERT_FALSE(o2.FuzzyEqualsWasCalled);
	}

	{
		ObjectWithFuzzyEquals o1(2);
		ObjectWithFuzzyEquals o2(1);

		ASSERT_TRUE(FuzzyEquals()(o1, o2));
		ASSERT_TRUE(o1.FuzzyEqualsWasCalled);
		ASSERT_FALSE(o2.FuzzyEqualsWasCalled);
	}
}
