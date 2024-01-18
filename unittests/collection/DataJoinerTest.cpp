// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/ByteData.h>
#include <stingraykit/collection/DataJoiner.h>

#include <algorithm>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	struct CounterVisitor
	{
		size_t Count;

		CounterVisitor() : Count(0)
		{}

		template<typename Iter>
		void operator()(const Iter& first, const Iter& last) { Count += std::distance(first, last); }
	};

}


TEST(DataJoinerTest, Basics)
{
	u8 data1_arr[] = {1, 2, 3};
	u8 data2_arr[] = {3, 4, 5, 6};
	u8 data3_arr[] = {1, 2, 3, 3, 4, 5, 6};

	ConstByteData data1(data1_arr, sizeof(data1_arr));
	ConstByteData data2(data2_arr, sizeof(data2_arr));
	ConstByteData data3(data3_arr, sizeof(data3_arr));

	typedef DataJoiner<TypeList<ConstByteData, ConstByteData> > JoinerType;
	JoinerType joiner(MakeTuple(&data1, &data2));

	ASSERT_EQ(joiner.size(), 7u);
	ASSERT_EQ(std::distance(joiner.begin(), joiner.end()), 7);
	JoinerType::iterator it = joiner.begin();
	ASSERT_EQ(*it, 1u);
	ASSERT_EQ(*(++it), 2u);
	ASSERT_TRUE(std::equal(joiner.begin(), joiner.end(), data3.begin()));

	std::vector<u8> v(data2.begin(), data2.end());
	typedef DataJoiner<TypeList<ConstByteData, std::vector<u8> > > Joiner2Type;
	Joiner2Type joiner2(MakeTuple(&data1, &v));

	ASSERT_EQ(joiner2.size(), 7u);
	ASSERT_TRUE(std::equal(joiner2.begin(), joiner2.end(), data3.begin()));
}


TEST(DataJoinerTest, Empty)
{
	u8 data1_arr[] = {1, 2, 3};
	u8 data2_arr[] = {3, 4, 5, 6};
	u8 data3_arr[] = {1, 2, 3, 3, 4, 5, 6};

	ConstByteData data1(data1_arr, sizeof(data1_arr));
	ConstByteData data2(data2_arr, sizeof(data2_arr));
	ConstByteData data3(data3_arr, sizeof(data3_arr));

	std::vector<u8> v(data1.begin(), data1.end());
	std::vector<u8> v2;
	typedef DataJoiner<TypeList<std::vector<u8>, std::vector<u8>, ConstByteData > > JoinerType;
	JoinerType joiner(MakeTuple(&v2, &v, &data2));

	ASSERT_EQ(joiner.size(), 7u);
	ASSERT_TRUE(std::equal(joiner.begin(), joiner.end(), data3.begin()));
	JoinerType::const_iterator it = std::find(joiner.begin(), joiner.end(), 2u);
	ASSERT_EQ(it - joiner.begin(), 1);
	ASSERT_EQ(*it, 2u);
}


TEST(DataJoinerTest, Visitor)
{
	u8 data1_arr[] = {1, 2, 3};
	u8 data2_arr[] = {3, 4, 5, 6};
	u8 data3_arr[] = {1, 2, 3, 3, 4, 5, 6};

	ConstByteData data1(data1_arr, sizeof(data1_arr));
	ConstByteData data2(data2_arr, sizeof(data2_arr));
	ConstByteData data3(data3_arr, sizeof(data3_arr));

	typedef DataJoiner<TypeList<ConstByteData, ConstByteData> > JoinerType;
	JoinerType joiner(MakeTuple(&data1, &data2));

	CounterVisitor v;
	joiner.ApplyToIntervals(joiner.begin(), joiner.end(), v);
	ASSERT_EQ(v.Count, 7u);

	CounterVisitor v2;
	joiner.ApplyToIntervals(joiner.begin() + 4, joiner.end() - 1, v2);
	ASSERT_EQ(v2.Count, 2u);
}
