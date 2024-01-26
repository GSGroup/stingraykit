// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/inplace_vector.h>

#include <gmock/gmock-matchers.h>

using namespace stingray;

using ::testing::ElementsAre;


TEST(InplaceVectorTest, Empty)
{
	inplace_vector<std::string, 5> testee;
	const auto& const_testee = testee;

	ASSERT_ANY_THROW(testee.at(0));
	ASSERT_ANY_THROW(testee.at(5));

	ASSERT_ANY_THROW(const_testee.at(0));
	ASSERT_ANY_THROW(const_testee.at(5));

	ASSERT_ANY_THROW(testee[0]);
	ASSERT_ANY_THROW(testee[5]);

	ASSERT_ANY_THROW(const_testee[0]);
	ASSERT_ANY_THROW(const_testee[5]);

	ASSERT_EQ(testee.begin(), testee.end());
	ASSERT_EQ(std::distance(testee.begin(), testee.end()), 0);

	ASSERT_EQ(const_testee.begin(), const_testee.end());
	ASSERT_EQ(std::distance(const_testee.begin(), const_testee.end()), 0);

	ASSERT_TRUE(testee.empty());
	ASSERT_EQ(testee.size(), 0);

	ASSERT_EQ(testee.capacity(), 5);
}


TEST(InplaceVectorTest, Assign)
{
	const std::string strs[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };

	inplace_vector<std::string, 5> testee;
	const auto& const_testee = testee;

	testee.assign(std::begin(strs), std::end(strs));

	ASSERT_EQ(testee.at(0), "0");
	ASSERT_EQ(testee.at(5), "5");
	ASSERT_ANY_THROW(testee.at(10));

	ASSERT_EQ(const_testee.at(0), "0");
	ASSERT_EQ(const_testee.at(5), "5");
	ASSERT_ANY_THROW(const_testee.at(10));

	ASSERT_EQ(testee[0], "0");
	ASSERT_EQ(testee[5], "5");
	ASSERT_ANY_THROW(testee[10]);

	ASSERT_EQ(const_testee[0], "0");
	ASSERT_EQ(const_testee[5], "5");
	ASSERT_ANY_THROW(const_testee[10]);

	testee.at(0) = "00";
	testee.at(5) = "55";

	ASSERT_EQ(testee.at(0), "00");
	ASSERT_EQ(testee.at(5), "55");

	testee[0] = "000";
	testee[5] = "555";

	ASSERT_EQ(testee[0], "000");
	ASSERT_EQ(testee[5], "555");

	ASSERT_NE(testee.begin(), testee.end());
	ASSERT_EQ(std::distance(testee.begin(), testee.end()), 10);

	ASSERT_NE(const_testee.begin(), const_testee.end());
	ASSERT_EQ(std::distance(const_testee.begin(), const_testee.end()), 10);

	ASSERT_FALSE(testee.empty());
	ASSERT_EQ(testee.size(), 10);

	ASSERT_GE(testee.capacity(), 10);

	ASSERT_THAT(testee, ElementsAre("000", "1", "2", "3", "4", "555", "6", "7", "8", "9"));
	ASSERT_THAT(const_testee, ElementsAre("000", "1", "2", "3", "4", "555", "6", "7", "8", "9"));
}


TEST(InplaceVectorTest, PushBack)
{
	inplace_vector<std::string, 2> testee;
	const auto& const_testee = testee;

	ASSERT_TRUE(testee.empty());
	ASSERT_EQ(testee.size(), 0);

	testee.push_back("0");

	ASSERT_FALSE(testee.empty());
	ASSERT_EQ(testee.size(), 1);

	testee.push_back("1");

	ASSERT_FALSE(testee.empty());
	ASSERT_EQ(testee.size(), 2);

	testee.push_back("2");

	ASSERT_FALSE(testee.empty());
	ASSERT_EQ(testee.size(), 3);

	testee.push_back("3");

	ASSERT_FALSE(testee.empty());
	ASSERT_EQ(testee.size(), 4);

	ASSERT_THAT(testee, ElementsAre("0", "1", "2", "3"));
	ASSERT_THAT(const_testee, ElementsAre("0", "1", "2", "3"));
}
