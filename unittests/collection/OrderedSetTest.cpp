// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/ordered_set.h>

#include <gmock/gmock-matchers.h>

using namespace stingray;

using ::testing::ElementsAre;

namespace
{

	using Vector = std::vector<std::string>;
	using Set = std::set<std::string>;
	using OrderedSet = ordered_set<std::string>;

}


TEST(OrderedSetTest, Construction)
{
	{
		OrderedSet testee;
		ASSERT_TRUE(testee.empty());
	}
	{
		OrderedSet testee((OrderedSet::key_compare()));
		ASSERT_TRUE(testee.empty());
	}
	{
		OrderedSet testee((OrderedSet::allocator_type()));
		ASSERT_TRUE(testee.empty());
	}
	{
		const Vector vec = { "2", "4", "3", "1", "1", "3" };

		OrderedSet testee1(vec.begin(), vec.end());
		OrderedSet testee2(vec.begin(), vec.end(), OrderedSet::allocator_type());

		ASSERT_EQ(testee1.size(), (size_t)4);
		ASSERT_EQ(testee2.size(), (size_t)4);

		ASSERT_TRUE(!std::is_sorted(testee1.begin(), testee1.end(), testee1.value_comp()));
		ASSERT_TRUE(!std::is_sorted(testee2.begin(), testee2.end(), testee2.key_comp()));

		ASSERT_THAT(testee1, ElementsAre("2", "4", "3", "1"));
		ASSERT_THAT(testee2, ElementsAre("2", "4", "3", "1"));
	}
	{
		const Set set = { "2", "4", "3", "1", "1", "3" };

		OrderedSet testee(set.begin(), set.end());

		ASSERT_TRUE(std::is_sorted(testee.begin(), testee.end(), testee.value_comp()));
		ASSERT_TRUE(std::equal(set.begin(), set.end(), testee.begin(), testee.end()));

		ASSERT_THAT(testee, ElementsAre("1", "2", "3", "4"));
	}
	{
		OrderedSet testee1{ "5", "1", "4", "2", "3" };

		OrderedSet testee2(testee1);
		ASSERT_FALSE(testee1.empty());

		OrderedSet testee3(testee1, OrderedSet::allocator_type());
		ASSERT_FALSE(testee1.empty());

		OrderedSet testee4(std::move(testee1));
		ASSERT_TRUE(testee1.empty());

		OrderedSet testee5(std::move(testee2), OrderedSet::allocator_type());
		ASSERT_TRUE(testee2.empty());

		ASSERT_TRUE(!std::is_sorted(testee3.begin(), testee3.end(), testee3.value_comp()));

		ASSERT_TRUE(std::equal(testee3.begin(), testee3.end(), testee4.begin(), testee4.end()));
		ASSERT_TRUE(std::equal(testee3.begin(), testee3.end(), testee5.begin(), testee5.end()));

		ASSERT_THAT(testee3, ElementsAre("5", "1", "4", "2", "3"));
	}
	{
		const std::initializer_list<OrderedSet::value_type> values{ "2", "4", "3", "1", "1", "3" };

		OrderedSet testee1(values);
		OrderedSet testee2(values, OrderedSet::allocator_type());

		ASSERT_TRUE(!std::is_sorted(testee1.begin(), testee1.end(), testee1.value_comp()));

		ASSERT_TRUE(std::equal(testee1.begin(), testee1.end(), testee2.begin(), testee2.end()));

		ASSERT_THAT(testee1, ElementsAre("2", "4", "3", "1"));
	}
}

TEST(OrderedSetTest, Assignment)
{
	{
		const OrderedSet testee1{ "5", "1", "4", "2", "3" };

		OrderedSet testee2;
		testee2 = testee1;

		ASSERT_TRUE(!std::is_sorted(testee2.begin(), testee2.end(), testee2.value_comp()));

		ASSERT_THAT(testee2, ElementsAre("5", "1", "4", "2", "3"));
	}
	{
		const std::initializer_list<OrderedSet::value_type> values{ "2", "4", "3", "1", "1", "3" };

		OrderedSet testee;
		testee = values;

		ASSERT_TRUE(!std::is_sorted(testee.begin(), testee.end(), testee.value_comp()));

		ASSERT_THAT(testee, ElementsAre("2", "4", "3", "1"));
	}
}

TEST(OrderedSetTest, Insertion)
{
	{
		OrderedSet testee;

		OrderedSet::value_type two("2");
		auto twoResult = testee.insert(two);
		ASSERT_EQ(two, "2");
		ASSERT_TRUE(twoResult.second);
		ASSERT_EQ(*twoResult.first, "2");

		OrderedSet::value_type four("4");
		auto fourResult = testee.insert(four);
		ASSERT_EQ(four, "4");
		ASSERT_TRUE(fourResult.second);
		ASSERT_EQ(*fourResult.first, "4");

		OrderedSet::value_type three("3");
		auto threeResult = testee.insert(three);
		ASSERT_EQ(three, "3");
		ASSERT_TRUE(threeResult.second);
		ASSERT_EQ(*threeResult.first, "3");

		OrderedSet::value_type one("1");
		auto oneResult = testee.insert(one);
		ASSERT_EQ(one, "1");
		ASSERT_TRUE(oneResult.second);
		ASSERT_EQ(*oneResult.first, "1");

		OrderedSet::value_type one2("1");
		auto one2Result = testee.insert(one2);
		ASSERT_EQ(one2, "1");
		ASSERT_FALSE(one2Result.second);
		ASSERT_EQ(*one2Result.first, "1");

		OrderedSet::value_type three2("3");
		auto three2Result = testee.insert(three2);
		ASSERT_EQ(three2, "3");
		ASSERT_FALSE(three2Result.second);
		ASSERT_EQ(*three2Result.first, "3");

		ASSERT_TRUE(!std::is_sorted(testee.begin(), testee.end(), testee.value_comp()));

		ASSERT_THAT(testee, ElementsAre("2", "4", "3", "1"));
	}
	{
		OrderedSet testee;

		OrderedSet::value_type two("2");
		auto twoResult = testee.insert(std::move(two));
		ASSERT_TRUE(two.empty());
		ASSERT_TRUE(twoResult.second);
		ASSERT_EQ(*twoResult.first, "2");

		OrderedSet::value_type four("4");
		auto fourResult = testee.insert(std::move(four));
		ASSERT_TRUE(four.empty());
		ASSERT_TRUE(fourResult.second);
		ASSERT_EQ(*fourResult.first, "4");

		OrderedSet::value_type three("3");
		auto threeResult = testee.insert(std::move(three));
		ASSERT_TRUE(three.empty());
		ASSERT_TRUE(threeResult.second);
		ASSERT_EQ(*threeResult.first, "3");

		OrderedSet::value_type one("1");
		auto oneResult = testee.insert(std::move(one));
		ASSERT_TRUE(one.empty());
		ASSERT_TRUE(oneResult.second);
		ASSERT_EQ(*oneResult.first, "1");

		OrderedSet::value_type one2("1");
		auto one2Result = testee.insert(std::move(one2));
		ASSERT_EQ(one2, "1");
		ASSERT_FALSE(one2Result.second);
		ASSERT_EQ(*one2Result.first, "1");

		OrderedSet::value_type three2("3");
		auto three2Result = testee.insert(std::move(three2));
		ASSERT_EQ(three2, "3");
		ASSERT_FALSE(three2Result.second);
		ASSERT_EQ(*three2Result.first, "3");

		ASSERT_TRUE(!std::is_sorted(testee.begin(), testee.end(), testee.value_comp()));

		ASSERT_THAT(testee, ElementsAre("2", "4", "3", "1"));
	}
	{
		OrderedSet testee;

		OrderedSet::value_type two("2");
		auto twoIt = testee.insert(testee.end(), two);
		ASSERT_EQ(two, "2");
		ASSERT_EQ(*twoIt, "2");

		OrderedSet::value_type four("4");
		auto fourIt = testee.insert(testee.end(), four);
		ASSERT_EQ(four, "4");
		ASSERT_EQ(*fourIt, "4");

		OrderedSet::value_type three("3");
		auto threeIt = testee.insert(testee.begin(), three);
		ASSERT_EQ(three, "3");
		ASSERT_EQ(*threeIt, "3");

		OrderedSet::value_type one("1");
		auto oneIt = testee.insert(testee.begin(), one);
		ASSERT_EQ(one, "1");
		ASSERT_EQ(*oneIt, "1");

		OrderedSet::value_type one2("1");
		auto one2It = testee.insert(testee.end(), one2);
		ASSERT_EQ(one2, "1");
		ASSERT_EQ(*one2It, "1");

		OrderedSet::value_type three2("3");
		auto three2It = testee.insert(testee.end(), three2);
		ASSERT_EQ(three2, "3");
		ASSERT_EQ(*three2It, "3");

		OrderedSet::const_iterator twoIt2 = testee.find("2");
		ASSERT_EQ(*twoIt2, "2");
		OrderedSet::value_type five("5");
		auto fiveIt = testee.insert(twoIt2, five);
		ASSERT_EQ(five, "5");
		ASSERT_EQ(*fiveIt, "5");

		ASSERT_TRUE(!std::is_sorted(testee.begin(), testee.end(), testee.value_comp()));

		ASSERT_THAT(testee, ElementsAre("1", "3", "5", "2", "4"));
	}
	{
		OrderedSet testee;

		OrderedSet::value_type two("2");
		auto twoIt = testee.insert(testee.end(), std::move(two));
		ASSERT_TRUE(two.empty());
		ASSERT_EQ(*twoIt, "2");

		OrderedSet::value_type four("4");
		auto fourIt = testee.insert(testee.end(), std::move(four));
		ASSERT_TRUE(four.empty());
		ASSERT_EQ(*fourIt, "4");

		OrderedSet::value_type three("3");
		auto threeIt = testee.insert(testee.begin(), std::move(three));
		ASSERT_TRUE(three.empty());
		ASSERT_EQ(*threeIt, "3");

		OrderedSet::value_type one("1");
		auto oneIt = testee.insert(testee.begin(), std::move(one));
		ASSERT_TRUE(one.empty());
		ASSERT_EQ(*oneIt, "1");

		OrderedSet::value_type one2("1");
		auto one2It = testee.insert(testee.end(), std::move(one2));
		ASSERT_EQ(one2, "1");
		ASSERT_EQ(*one2It, "1");

		OrderedSet::value_type three2("3");
		auto three2It = testee.insert(testee.end(), std::move(three2));
		ASSERT_EQ(three2, "3");
		ASSERT_EQ(*three2It, "3");

		OrderedSet::const_iterator twoIt2 = testee.find("2");
		ASSERT_EQ(*twoIt2, "2");
		OrderedSet::value_type five("5");
		auto fiveIt = testee.insert(twoIt2, std::move(five));
		ASSERT_TRUE(five.empty());

		ASSERT_TRUE(!std::is_sorted(testee.begin(), testee.end(), testee.value_comp()));

		ASSERT_THAT(testee, ElementsAre("1", "3", "5", "2", "4"));
	}
}

TEST(OrderedSetTest, Emplacing)
{
	{
		OrderedSet testee;

		OrderedSet::value_type two("2");
		auto twoResult = testee.emplace(std::move(two));
		ASSERT_TRUE(two.empty());
		ASSERT_TRUE(twoResult.second);
		ASSERT_EQ(*twoResult.first, "2");

		OrderedSet::value_type four("4");
		auto fourResult = testee.emplace(std::move(four));
		ASSERT_TRUE(four.empty());
		ASSERT_TRUE(fourResult.second);
		ASSERT_EQ(*fourResult.first, "4");

		OrderedSet::value_type three("3");
		auto threeResult = testee.emplace(std::move(three));
		ASSERT_TRUE(three.empty());
		ASSERT_TRUE(threeResult.second);
		ASSERT_EQ(*threeResult.first, "3");

		OrderedSet::value_type one("1");
		auto oneResult = testee.emplace(std::move(one));
		ASSERT_TRUE(one.empty());
		ASSERT_TRUE(oneResult.second);
		ASSERT_EQ(*oneResult.first, "1");

		OrderedSet::value_type one2("1");
		auto one2Result = testee.emplace(std::move(one2));
		ASSERT_TRUE(one2.empty());
		ASSERT_FALSE(one2Result.second);
		ASSERT_EQ(*one2Result.first, "1");

		OrderedSet::value_type three2("3");
		auto three2Result = testee.emplace(std::move(three2));
		ASSERT_TRUE(three2.empty());
		ASSERT_FALSE(three2Result.second);
		ASSERT_EQ(*three2Result.first, "3");

		ASSERT_TRUE(!std::is_sorted(testee.begin(), testee.end(), testee.value_comp()));

		ASSERT_THAT(testee, ElementsAre("2", "4", "3", "1"));
	}
	{
		OrderedSet testee;

		OrderedSet::value_type two("2");
		auto twoIt = testee.emplace_hint(testee.end(), std::move(two));
		ASSERT_TRUE(two.empty());
		ASSERT_EQ(*twoIt, "2");

		OrderedSet::value_type four("4");
		auto fourIt = testee.emplace_hint(testee.end(), std::move(four));
		ASSERT_TRUE(four.empty());
		ASSERT_EQ(*fourIt, "4");

		OrderedSet::value_type three("3");
		auto threeIt = testee.emplace_hint(testee.begin(), std::move(three));
		ASSERT_TRUE(three.empty());
		ASSERT_EQ(*threeIt, "3");

		OrderedSet::value_type one("1");
		auto oneIt = testee.emplace_hint(testee.begin(), std::move(one));
		ASSERT_TRUE(one.empty());
		ASSERT_EQ(*oneIt, "1");

		OrderedSet::value_type one2("1");
		auto one2It = testee.emplace_hint(testee.end(), std::move(one2));
		ASSERT_TRUE(one2.empty());
		ASSERT_EQ(*one2It, "1");

		OrderedSet::value_type three2("3");
		auto three2It = testee.emplace_hint(testee.end(), std::move(three2));
		ASSERT_TRUE(three2.empty());
		ASSERT_EQ(*three2It, "3");

		OrderedSet::const_iterator twoIt2 = testee.find("2");
		ASSERT_EQ(*twoIt2, "2");
		OrderedSet::key_type five("5");
		auto fiveIt = testee.emplace_hint(twoIt2, std::move(five));
		ASSERT_TRUE(five.empty());
		ASSERT_EQ(*fiveIt, "5");

		ASSERT_TRUE(!std::is_sorted(testee.begin(), testee.end(), testee.value_comp()));

		ASSERT_THAT(testee, ElementsAre("1", "3", "5", "2", "4"));
	}
}

TEST(OrderedSetTest, Lookup)
{
	const Vector unsorted{ "5", "4", "8", "9", "1", "6", "3", "2", "7", "0" };

	const OrderedSet testee(unsorted.begin(), unsorted.end());
	const Set sample(unsorted.begin(), unsorted.end());

	ASSERT_TRUE(!std::is_sorted(testee.begin(), testee.end(), testee.value_comp()));

	for (Set::const_iterator sampleIt = sample.begin(); sampleIt != sample.end(); ++sampleIt)
	{
		OrderedSet::const_iterator testeeIt = testee.find(*sampleIt);
		ASSERT_NE(testeeIt, testee.end());
		ASSERT_TRUE(*testeeIt == *sampleIt);
	}

	for (OrderedSet::const_iterator testeeIt = testee.begin(); testeeIt != testee.end(); ++testeeIt)
	{
		Set::const_iterator sampleIt = sample.find(*testeeIt);
		ASSERT_NE(sampleIt, sample.end());
		ASSERT_TRUE(*sampleIt == *testeeIt);
	}

	for (Set::const_reverse_iterator sampleIt = sample.rbegin(); sampleIt != sample.rend(); ++sampleIt)
	{
		OrderedSet::const_iterator testeeIt = testee.find(*sampleIt);
		ASSERT_NE(testeeIt, testee.end());
		ASSERT_TRUE(*testeeIt == *sampleIt);
	}

	for (OrderedSet::const_reverse_iterator testeeIt = testee.rbegin(); testeeIt != testee.rend(); ++testeeIt)
	{
		Set::const_iterator sampleIt = sample.find(*testeeIt);
		ASSERT_NE(sampleIt, sample.end());
		ASSERT_TRUE(*sampleIt == *testeeIt);
	}

	for (Set::const_iterator sampleIt = sample.begin(); sampleIt != sample.end(); ++sampleIt)
		ASSERT_EQ(testee.count(*sampleIt), 1);

	for (OrderedSet::const_iterator testeeIt = testee.begin(); testeeIt != testee.end(); ++testeeIt)
		ASSERT_EQ(sample.count(*testeeIt), 1);
}

TEST(OrderedSetTest, Swap)
{
	{
		OrderedSet testee1{ "5", "1", "4", "2", "3" };
		OrderedSet testee2;

		testee1.swap(testee2);

		ASSERT_TRUE(testee1.empty());
		ASSERT_THAT(testee2, ElementsAre("5", "1", "4", "2", "3"));
	}
}

TEST(OrderedSetTest, Removal)
{
	{
		OrderedSet testee{ "5", "1", "4", "2", "3" };

		ASSERT_TRUE(testee.count("4"));
		ASSERT_TRUE(testee.erase("4"));
		ASSERT_FALSE(testee.count("4"));
		ASSERT_FALSE(testee.erase("4"));

		ASSERT_TRUE(testee.count("5"));
		ASSERT_TRUE(testee.erase("5"));
		ASSERT_FALSE(testee.count("5"));
		ASSERT_FALSE(testee.erase("5"));

		ASSERT_TRUE(testee.count("3"));
		ASSERT_TRUE(testee.erase("3"));
		ASSERT_FALSE(testee.count("3"));
		ASSERT_FALSE(testee.erase("3"));

		ASSERT_THAT(testee, ElementsAre("1", "2"));
	}
	{
		OrderedSet testee{ "5", "1", "4", "2", "3" };

		OrderedSet::const_iterator fourIt = testee.find("4");
		ASSERT_NE(fourIt, testee.end());
		ASSERT_EQ(*fourIt, "4");
		OrderedSet::const_iterator twoIt = testee.erase(fourIt);
		ASSERT_NE(twoIt, testee.end());
		ASSERT_EQ(*twoIt, "2");
		ASSERT_FALSE(testee.count("4"));

		OrderedSet::const_iterator fiveIt = testee.find("5");
		ASSERT_NE(fiveIt, testee.end());
		ASSERT_EQ(*fiveIt, "5");
		OrderedSet::const_iterator oneIt = testee.erase(fiveIt);
		ASSERT_NE(oneIt, testee.end());
		ASSERT_EQ(*oneIt, "1");
		ASSERT_FALSE(testee.count("5"));

		OrderedSet::const_iterator threeIt = testee.find("3");
		ASSERT_NE(threeIt, testee.end());
		ASSERT_EQ(*threeIt, "3");
		OrderedSet::const_iterator endIt = testee.erase(threeIt);
		ASSERT_EQ(endIt, testee.end());
		ASSERT_FALSE(testee.count("5"));

		ASSERT_THAT(testee, ElementsAre("1", "2"));
	}
	{
		OrderedSet testee{ "5", "1", "4", "2", "3" };

		OrderedSet::const_iterator oneIt = testee.find("1");
		ASSERT_NE(oneIt, testee.end());
		ASSERT_EQ(*oneIt, "1");
		OrderedSet::const_iterator twoIt = testee.find("2");
		ASSERT_NE(twoIt, testee.end());
		ASSERT_EQ(*twoIt, "2");
		OrderedSet::const_iterator two2It = testee.erase(oneIt, twoIt);
		ASSERT_NE(two2It, testee.end());
		ASSERT_EQ(*two2It, "2");
		ASSERT_FALSE(testee.count("1"));
		ASSERT_FALSE(testee.count("4"));
		ASSERT_TRUE(testee.count("2"));

		ASSERT_THAT(testee, ElementsAre("5", "2", "3"));

		OrderedSet::const_iterator endIt = testee.erase(testee.begin(), testee.end());
		ASSERT_EQ(endIt, testee.end());

		ASSERT_TRUE(testee.empty());
	}
}

TEST(OrderedSetTest, Comparison)
{
	{
		const OrderedSet testee1{ "5", "1", "4", "2", "3" };
		const OrderedSet testee2(testee1);

		ASSERT_TRUE(testee1 == testee2);
		ASSERT_FALSE(testee1 != testee2);
		ASSERT_FALSE(testee1 < testee2);
		ASSERT_FALSE(testee1 > testee2);
		ASSERT_TRUE(testee1 <= testee2);
		ASSERT_TRUE(testee1 >= testee2);
	}
	{
		const OrderedSet testee1{ "5", "1", "4", "2", "3" };
		const OrderedSet testee2{ "5", "1", "4", "2" };

		ASSERT_FALSE(testee1 == testee2);
		ASSERT_TRUE(testee1 != testee2);
		ASSERT_FALSE(testee1 < testee2);
		ASSERT_TRUE(testee1 > testee2);
		ASSERT_FALSE(testee1 <= testee2);
		ASSERT_TRUE(testee1 >= testee2);

		ASSERT_FALSE(testee2 == testee1);
		ASSERT_TRUE(testee2 != testee1);
		ASSERT_TRUE(testee2 < testee1);
		ASSERT_FALSE(testee2 > testee1);
		ASSERT_TRUE(testee2 <= testee1);
		ASSERT_FALSE(testee2 >= testee1);
	}
	{
		const OrderedSet testee1{ "5", "4", "1", "2", "3" };
		const OrderedSet testee2{ "5", "1", "4", "2", "3" };

		ASSERT_FALSE(testee1 == testee2);
		ASSERT_TRUE(testee1 != testee2);
		ASSERT_FALSE(testee1 < testee2);
		ASSERT_TRUE(testee1 > testee2);
		ASSERT_FALSE(testee1 <= testee2);
		ASSERT_TRUE(testee1 >= testee2);

		ASSERT_FALSE(testee2 == testee1);
		ASSERT_TRUE(testee2 != testee1);
		ASSERT_TRUE(testee2 < testee1);
		ASSERT_FALSE(testee2 > testee1);
		ASSERT_TRUE(testee2 <= testee1);
		ASSERT_FALSE(testee2 >= testee1);
	}
}
