#include <stingraykit/compare/MemberListComparer.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{
	struct Foo
	{
		bool a;
		void *b;
		int c;

		inline Foo(bool a = false, void *b = 0, int c = 0): a(a), b(b), c(c) {}

		inline bool operator < (const Foo& other) const
		{ return CompareMembersLess(&Foo::a, &Foo::b, &Foo::c)(*this, other); }

		inline bool operator == (const Foo& other) const
		{ return CompareMembersEquals(&Foo::a, &Foo::b, &Foo::c)(*this, other); }

	};
}


TEST(MemberCompareTest, First)
{
	Foo l, r;
	ASSERT_EQ(l == r, true);
	ASSERT_EQ(l < r, false);

	r.a = true;
	r.c = -10;
	ASSERT_EQ(l == r, false);
	ASSERT_EQ(l < r, true);

	ASSERT_EQ(r == l, false);
	ASSERT_EQ(r < l, false);
}


TEST(MemberCompareTest, Third)
{
	Foo l, r;
	ASSERT_EQ(l == r, true);
	ASSERT_EQ(l < r, false);

	r.c = 10;
	ASSERT_EQ(l == r, false);
	ASSERT_EQ(l < r, true);

	l.c = 15;
	ASSERT_EQ(l == r, false);
	ASSERT_EQ(l < r, false);
}
