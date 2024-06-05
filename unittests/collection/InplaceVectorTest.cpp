// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/Range.h>

#include <unittests/RangeMatcher.h>

#include <gmock/gmock-matchers.h>

using namespace stingray;

using ::testing::ElementsAre;


namespace
{

	const std::string ThrowOnCreation = "ThrowOnCreation";
	const std::string ThrowOnCopying = "ThrowOnCopying";
	const std::string ThrowOnMoving = "ThrowOnMoving";

	struct ThrowableTestObject
	{
		std::string			Action;

		explicit ThrowableTestObject(const std::string& action)
			:	Action(action)
		{ STINGRAYKIT_CHECK(Action != ThrowOnCreation, Action); }

		ThrowableTestObject(const ThrowableTestObject& other)
			:	Action(other.Action)
		{ STINGRAYKIT_CHECK(Action != ThrowOnCopying, Action); }

		ThrowableTestObject(ThrowableTestObject&& other)
			:	Action(other.Action)
		{ STINGRAYKIT_CHECK(Action != ThrowOnMoving, Action); }

		bool operator == (const ThrowableTestObject& other) const
		{ return Action == other.Action; }
	};

}


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

	ASSERT_EQ(testee.cbegin(), testee.cend());
	ASSERT_EQ(std::distance(testee.cbegin(), testee.cend()), 0);

	ASSERT_EQ(testee.rbegin(), testee.rend());
	ASSERT_EQ(std::distance(testee.rbegin(), testee.rend()), 0);

	ASSERT_EQ(const_testee.rbegin(), const_testee.rend());
	ASSERT_EQ(std::distance(const_testee.rbegin(), const_testee.rend()), 0);

	ASSERT_EQ(testee.crbegin(), testee.crend());
	ASSERT_EQ(std::distance(testee.crbegin(), testee.crend()), 0);

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

	ASSERT_EQ(testee.front(), "000");
	ASSERT_EQ(const_testee.front(), "000");

	ASSERT_EQ(testee.back(), "9");
	ASSERT_EQ(const_testee.back(), "9");

	ASSERT_NE(testee.begin(), testee.end());
	ASSERT_EQ(std::distance(testee.begin(), testee.end()), 10);

	ASSERT_NE(const_testee.begin(), const_testee.end());
	ASSERT_EQ(std::distance(const_testee.begin(), const_testee.end()), 10);

	ASSERT_NE(testee.cbegin(), testee.cend());
	ASSERT_EQ(std::distance(testee.cbegin(), testee.cend()), 10);

	ASSERT_NE(testee.rbegin(), testee.rend());
	ASSERT_EQ(std::distance(testee.rbegin(), testee.rend()), 10);

	ASSERT_NE(const_testee.rbegin(), const_testee.rend());
	ASSERT_EQ(std::distance(const_testee.rbegin(), const_testee.rend()), 10);

	ASSERT_NE(testee.crbegin(), testee.crend());
	ASSERT_EQ(std::distance(testee.crbegin(), testee.crend()), 10);

	ASSERT_FALSE(testee.empty());
	ASSERT_EQ(testee.size(), 10);

	ASSERT_GE(testee.capacity(), 10);

	ASSERT_THAT(testee, ElementsAre("000", "1", "2", "3", "4", "555", "6", "7", "8", "9"));
	ASSERT_THAT(const_testee, ElementsAre("000", "1", "2", "3", "4", "555", "6", "7", "8", "9"));
	ASSERT_THAT(ToRange(testee.cbegin(), testee.cend()), MatchRange(ElementsAre("000", "1", "2", "3", "4", "555", "6", "7", "8", "9")));

	ASSERT_THAT(ToRange(testee.rbegin(), testee.rend()), MatchRange(ElementsAre("9", "8", "7", "6", "555", "4", "3", "2", "1", "000")));
	ASSERT_THAT(ToRange(const_testee.rbegin(), const_testee.rend()), MatchRange(ElementsAre("9", "8", "7", "6", "555", "4", "3", "2", "1", "000")));
	ASSERT_THAT(ToRange(testee.crbegin(), testee.crend()), MatchRange(ElementsAre("9", "8", "7", "6", "555", "4", "3", "2", "1", "000")));
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
	ASSERT_THAT(ToRange(testee.cbegin(), testee.cend()), MatchRange(ElementsAre("0", "1", "2", "3")));

	ASSERT_THAT(ToRange(testee.rbegin(), testee.rend()), MatchRange(ElementsAre("3", "2", "1", "0")));
	ASSERT_THAT(ToRange(const_testee.rbegin(), const_testee.rend()), MatchRange(ElementsAre("3", "2", "1", "0")));
	ASSERT_THAT(ToRange(testee.crbegin(), testee.crend()), MatchRange(ElementsAre("3", "2", "1", "0")));
}


TEST(InplaceVectorTest, PushBackException)
{
	inplace_vector<ThrowableTestObject, 2> testee;

	ASSERT_EQ(testee.size(), 0);

	ASSERT_NO_THROW(testee.push_back(ThrowableTestObject("0")));
	ASSERT_EQ(testee.size(), 1);

	{
		ThrowableTestObject throwable(ThrowOnCopying);
		ASSERT_ANY_THROW(testee.push_back(throwable));
		ASSERT_EQ(testee.size(), 1);
	}

	ASSERT_ANY_THROW(testee.push_back(ThrowableTestObject(ThrowOnMoving)));
	ASSERT_EQ(testee.size(), 1);

	ASSERT_NO_THROW(testee.push_back(ThrowableTestObject("1")));
	ASSERT_EQ(testee.size(), 2);

	ASSERT_NO_THROW(testee.push_back(ThrowableTestObject("2")));
	ASSERT_EQ(testee.size(), 3);

	{
		ThrowableTestObject throwable(ThrowOnCopying);
		ASSERT_ANY_THROW(testee.push_back(throwable));
		ASSERT_EQ(testee.size(), 3);
	}

	ASSERT_ANY_THROW(testee.push_back(ThrowableTestObject(ThrowOnMoving)));
	ASSERT_EQ(testee.size(), 3);

	ASSERT_NO_THROW(testee.push_back(ThrowableTestObject("3")));
	ASSERT_EQ(testee.size(), 4);

	ASSERT_THAT(testee, ElementsAre(ThrowableTestObject("0"), ThrowableTestObject("1"), ThrowableTestObject("2"), ThrowableTestObject("3")));
}


TEST(InplaceVectorTest, EmplaceBack)
{
	inplace_vector<std::string, 2> testee;
	const auto& const_testee = testee;

	ASSERT_TRUE(testee.empty());
	ASSERT_EQ(testee.size(), 0);

	testee.emplace_back("0");

	ASSERT_FALSE(testee.empty());
	ASSERT_EQ(testee.size(), 1);

	testee.emplace_back("1");

	ASSERT_FALSE(testee.empty());
	ASSERT_EQ(testee.size(), 2);

	testee.emplace_back("2");

	ASSERT_FALSE(testee.empty());
	ASSERT_EQ(testee.size(), 3);

	testee.emplace_back("3");

	ASSERT_FALSE(testee.empty());
	ASSERT_EQ(testee.size(), 4);

	ASSERT_THAT(testee, ElementsAre("0", "1", "2", "3"));
	ASSERT_THAT(const_testee, ElementsAre("0", "1", "2", "3"));
	ASSERT_THAT(ToRange(testee.cbegin(), testee.cend()), MatchRange(ElementsAre("0", "1", "2", "3")));

	ASSERT_THAT(ToRange(testee.rbegin(), testee.rend()), MatchRange(ElementsAre("3", "2", "1", "0")));
	ASSERT_THAT(ToRange(const_testee.rbegin(), const_testee.rend()), MatchRange(ElementsAre("3", "2", "1", "0")));
	ASSERT_THAT(ToRange(testee.crbegin(), testee.crend()), MatchRange(ElementsAre("3", "2", "1", "0")));
}


TEST(InplaceVectorTest, EmplaceBackException)
{
	inplace_vector<ThrowableTestObject, 2> testee;

	ASSERT_EQ(testee.size(), 0);

	ASSERT_NO_THROW(testee.emplace_back("0"));
	ASSERT_EQ(testee.size(), 1);

	ASSERT_ANY_THROW(testee.emplace_back(ThrowOnCreation));
	ASSERT_EQ(testee.size(), 1);

	ASSERT_NO_THROW(testee.emplace_back("1"));
	ASSERT_EQ(testee.size(), 2);

	ASSERT_NO_THROW(testee.emplace_back("2"));
	ASSERT_EQ(testee.size(), 3);

	ASSERT_ANY_THROW(testee.emplace_back(ThrowOnCreation));
	ASSERT_EQ(testee.size(), 3);

	ASSERT_NO_THROW(testee.emplace_back("3"));
	ASSERT_EQ(testee.size(), 4);

	ASSERT_THAT(testee, ElementsAre(ThrowableTestObject("0"), ThrowableTestObject("1"), ThrowableTestObject("2"), ThrowableTestObject("3")));
}
