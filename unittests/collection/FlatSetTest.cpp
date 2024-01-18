// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/flat_set.h>

#include <gmock/gmock-matchers.h>

using namespace stingray;

using ::testing::ElementsAre;

namespace
{

	using Vector = std::vector<std::string>;
	using Set = std::set<std::string>;
	using FlatSet = flat_set<std::string>;

	Vector GetUnorderedVector()
	{ return { "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten" }; }

	FlatSet GetSampleFlatSet()
	{ return { "Americans", "Australians", "Japaneses", "Russians", "Ukrainians" }; }

}


TEST(FlatSetTest, Construction)
{
	{
		FlatSet testee;
		EXPECT_TRUE(testee.empty());
	}
	{
		FlatSet testee((FlatSet::key_compare()));
		EXPECT_TRUE(testee.empty());
	}
	{
		FlatSet testee((FlatSet::key_compare()), (FlatSet::allocator_type()));
		EXPECT_TRUE(testee.empty());
	}
	{
		FlatSet testee((FlatSet::allocator_type()));
		EXPECT_TRUE(testee.empty());
	}
	{
		const Vector vec = { "one", "two", "three", "four", "four", "three" };

		FlatSet testee(vec.begin(), vec.end());
		EXPECT_EQ(testee.size(), (size_t)4);
		EXPECT_TRUE(std::is_sorted(testee.begin(), testee.end(), testee.key_comp()));

		ASSERT_THAT(testee, ElementsAre("four", "one", "three", "two"));
	}
	{
		const Set set = { "one", "two", "three", "four", "four", "three" };

		FlatSet testee(set.begin(), set.end());
		EXPECT_TRUE(std::is_sorted(testee.begin(), testee.end(), testee.key_comp()));
		EXPECT_TRUE(std::equal(set.begin(), set.end(), testee.begin(), testee.end()));

		ASSERT_THAT(testee, ElementsAre("four", "one", "three", "two"));
	}
	{
		FlatSet testee1 = GetSampleFlatSet();

		FlatSet testee2(testee1, FlatSet::allocator_type());
		EXPECT_FALSE(testee1.empty());

		FlatSet testee3(std::move(testee1), FlatSet::allocator_type());
		EXPECT_TRUE(testee1.empty());

		EXPECT_TRUE(std::equal(testee2.begin(), testee2.end(), testee3.begin(), testee3.end()));

		ASSERT_THAT(testee2, ElementsAre("Americans", "Australians", "Japaneses", "Russians", "Ukrainians"));
	}
}

TEST(FlatSetTest, Assignment)
{
	{
		const FlatSet testee1 = GetSampleFlatSet();

		FlatSet testee2;
		FlatSet testee3;
		testee2 = testee1;
		testee3 = testee1;

		EXPECT_TRUE(std::equal(testee2.begin(), testee2.end(), testee3.begin(), testee3.end()));

		ASSERT_THAT(testee2, ElementsAre("Americans", "Australians", "Japaneses", "Russians", "Ukrainians"));
	}
	{
		const std::initializer_list<std::string> values{ "one", "two", "three", "four", "four", "three" };

		FlatSet testee;
		testee = values;

		EXPECT_TRUE(std::is_sorted(testee.begin(), testee.end(), testee.key_comp()));

		ASSERT_THAT(testee, ElementsAre("four", "one", "three", "two"));
	}
}

TEST(FlatSetTest, Insertion)
{
	{
		FlatSet testee;
		EXPECT_TRUE(testee.insert("one").second);
		EXPECT_TRUE(testee.insert("two").second);
		EXPECT_TRUE(testee.insert("three").second);
		EXPECT_TRUE(testee.insert("four").second);
		EXPECT_FALSE(testee.insert("four").second);
		EXPECT_FALSE(testee.insert("three").second);

		EXPECT_TRUE(std::is_sorted(testee.begin(), testee.end(), testee.key_comp()));

		ASSERT_THAT(testee, ElementsAre("four", "one", "three", "two"));
	}
	{
		FlatSet testee;

		std::string one("one");
		EXPECT_TRUE(testee.insert(std::move(one)).second);
		EXPECT_TRUE(one.empty());

		std::string two("two");
		EXPECT_TRUE(testee.insert(std::move(two)).second);
		EXPECT_TRUE(two.empty());

		std::string three("three");
		EXPECT_TRUE(testee.insert(std::move(three)).second);
		EXPECT_TRUE(three.empty());

		std::string four("four");
		EXPECT_TRUE(testee.insert(std::move(four)).second);
		EXPECT_TRUE(four.empty());

		std::string four2("four");
		EXPECT_FALSE(testee.insert(std::move(four2)).second);
		EXPECT_FALSE(four2.empty());

		std::string three2("three");
		EXPECT_FALSE(testee.insert(std::move(three2)).second);
		EXPECT_FALSE(three2.empty());

		EXPECT_TRUE(std::is_sorted(testee.begin(), testee.end(), testee.key_comp()));

		ASSERT_THAT(testee, ElementsAre("four", "one", "three", "two"));
	}
	{
		FlatSet testee;

		std::string one("one");
		testee.insert(testee.end(), std::move(one));
		EXPECT_TRUE(one.empty());

		std::string two("two");
		testee.insert(testee.end(), std::move(two));
		EXPECT_TRUE(two.empty());

		std::string three("three");
		testee.insert(testee.begin(), std::move(three));
		EXPECT_TRUE(three.empty());

		std::string four("four");
		testee.insert(testee.begin(), std::move(four));
		EXPECT_TRUE(four.empty());

		std::string four2("four");
		testee.insert(testee.end(), std::move(four2));
		EXPECT_FALSE(four2.empty());

		std::string three2("three");
		testee.insert(testee.end(), std::move(three2));
		EXPECT_FALSE(three2.empty());

		EXPECT_TRUE(std::is_sorted(testee.begin(), testee.end(), testee.key_comp()));

		ASSERT_THAT(testee, ElementsAre("four", "one", "three", "two"));
	}
}

TEST(FlatSetTest, Emplacing)
{
	{
		FlatSet testee;
		EXPECT_TRUE(testee.emplace("one").second);
		EXPECT_TRUE(testee.emplace("two").second);
		EXPECT_TRUE(testee.emplace("three").second);
		EXPECT_TRUE(testee.emplace("four").second);
		EXPECT_FALSE(testee.emplace("four").second);
		EXPECT_FALSE(testee.emplace("three").second);

		EXPECT_TRUE(std::is_sorted(testee.begin(), testee.end(), testee.key_comp()));

		ASSERT_THAT(testee, ElementsAre("four", "one", "three", "two"));
	}
	{
		FlatSet testee;

		testee.emplace_hint(testee.end(), "one");
		testee.emplace_hint(testee.end(), "two");
		testee.emplace_hint(testee.begin(), "three");
		testee.emplace_hint(testee.begin(), "four");
		testee.emplace_hint(testee.end(), "four");
		testee.emplace_hint(testee.end(), "three");

		EXPECT_TRUE(std::is_sorted(testee.begin(), testee.end(), testee.key_comp()));

		ASSERT_THAT(testee, ElementsAre("four", "one", "three", "two"));
	}
}

TEST(FlatSetTest, Lookup)
{
	Vector unordered = GetUnorderedVector();

	FlatSet testee;
	Set sample;

	testee.reserve(unordered.size());
	for (Vector::const_iterator it = unordered.begin(); it != unordered.end(); ++it)
	{
		testee.insert(*it);
		sample.insert(*it);
	}

	EXPECT_TRUE(std::is_sorted(testee.begin(), testee.end(), testee.key_comp()));
	EXPECT_TRUE(std::equal(sample.begin(), sample.end(), testee.begin(), testee.end()));

	for (Set::const_iterator sample_iter = sample.begin(); sample_iter != sample.end(); ++sample_iter)
	{
		FlatSet::const_iterator testee_iter = testee.find(*sample_iter);
		ASSERT_NE(testee_iter, testee.end());
		EXPECT_TRUE(*testee_iter == *sample_iter);
	}
	for (FlatSet::const_iterator testee_iter = testee.begin(); testee_iter != testee.end(); ++testee_iter)
	{
		Set::const_iterator sample_iter = sample.find(*testee_iter);
		ASSERT_NE(sample_iter, sample.end());
		EXPECT_TRUE(*sample_iter == *testee_iter);
	}

	for (Set::const_reverse_iterator sample_iter = sample.rbegin(); sample_iter != sample.rend(); ++sample_iter)
	{
		FlatSet::const_iterator testee_iter = testee.find(*sample_iter);
		ASSERT_NE(testee_iter, testee.end());
		EXPECT_TRUE(*testee_iter == *sample_iter);
	}

	for (FlatSet::const_reverse_iterator testee_iter = testee.rbegin(); testee_iter != testee.rend(); ++testee_iter)
	{
		Set::const_iterator sample_iter = sample.find(*testee_iter);
		ASSERT_NE(sample_iter, sample.end());
		EXPECT_TRUE(*sample_iter == *testee_iter);
	}

	for (Set::const_iterator sample_iter = sample.begin(); sample_iter != sample.end(); ++sample_iter)
		ASSERT_TRUE(testee.count(*sample_iter));
	for (FlatSet::const_iterator testee_iter = testee.begin(); testee_iter != testee.end(); ++testee_iter)
		ASSERT_TRUE(sample.count(*testee_iter));
}

TEST(FlatSetTest, Removal)
{
	{
		FlatSet testee(GetSampleFlatSet());
		ASSERT_TRUE(testee.count("Ukrainians"));
		ASSERT_TRUE(testee.erase("Ukrainians"));
		ASSERT_FALSE(testee.erase("Ukrainians"));
	}
	{
		FlatSet testee(GetSampleFlatSet());
		FlatSet::const_iterator first = testee.find("Russians");
		ASSERT_TRUE(first != testee.end());
		FlatSet::const_iterator next = testee.erase(first);
		ASSERT_EQ(*next, "Ukrainians");
		ASSERT_FALSE(testee.count("Russians"));
	}
	{
		FlatSet testee(GetSampleFlatSet());
		FlatSet::const_iterator first = testee.find("Americans");
		FlatSet::const_iterator last = testee.find("Japaneses");
		ASSERT_TRUE(first != testee.end());
		ASSERT_TRUE(last != testee.end());
		FlatSet::const_iterator next = testee.erase(first, last);
		ASSERT_EQ(*next, "Japaneses");
		ASSERT_FALSE(testee.erase("Americans"));
		ASSERT_TRUE(testee.count("Japaneses"));
	}
}

