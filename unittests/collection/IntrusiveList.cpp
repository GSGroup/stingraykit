// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/IntrusiveList.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	struct Node : public IntrusiveListNode<Node>
	{
	private:
		int _value;

	public:
		Node(int val) : _value(val)
		{ }

		int Get() const
		{ return _value; }
	};
	typedef IntrusiveList<Node> List;

}

TEST(IntrusiveListTest, Test)
{
	List x;
	ASSERT_TRUE(x.empty());
	ASSERT_TRUE(x.begin() == x.end());
	ASSERT_EQ(x.size(), 0u);

	Node n0(0);
	x.push_back(n0);
	ASSERT_TRUE(!x.empty());
	ASSERT_TRUE(x.begin() != x.end());
	ASSERT_EQ(x.size(), 1u);
	{
		List::iterator it0 = x.begin();
		List::iterator iend = std::next(it0);
		ASSERT_EQ(it0->Get(), 0);
		ASSERT_TRUE(&(*it0) == &n0);
		ASSERT_TRUE(iend == x.end());
	}

	Node n1(1);
	x.push_back(n1);
	ASSERT_TRUE(!x.empty());
	ASSERT_TRUE(x.begin() != x.end());
	ASSERT_EQ(x.size(), 2u);
	{
		List::iterator it0 = x.begin();
		List::iterator it1 = std::next(it0);
		List::iterator iend = std::next(it1);
		ASSERT_EQ(it0->Get(), 0);
		ASSERT_TRUE(&(*it0) == &n0);
		ASSERT_EQ(it1->Get(), 1);
		ASSERT_TRUE(&(*it1) == &n1);
		ASSERT_TRUE(iend == x.end());
	}

	x.erase(n0);
	ASSERT_EQ(x.empty(), false);
	ASSERT_EQ(x.begin() == x.end(), false);
	ASSERT_EQ(x.size(), 1u);
	{
		List::iterator it0 = x.begin();
		List::iterator iend = std::next(it0);
		ASSERT_EQ(it0->Get(), 1);
		ASSERT_TRUE(&(*it0) == &n1);
		ASSERT_TRUE(iend == x.end());
	}

	x.erase(n1);
	ASSERT_TRUE(x.empty());
	ASSERT_TRUE(x.begin() == x.end());
	ASSERT_EQ(x.size(), 0u);
}
