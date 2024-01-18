// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

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
