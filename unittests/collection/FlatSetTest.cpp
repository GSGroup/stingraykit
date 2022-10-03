#include <stingraykit/collection/flat_set.h>

#include <gtest/gtest.h>

using namespace stingray;

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
		const Vector vec = { "one", "two", "three", "four", "four", "three" };

		FlatSet testee(vec.begin(), vec.end());
		EXPECT_EQ(testee.size(), (size_t)4);
		EXPECT_TRUE(std::is_sorted(testee.begin(), testee.end(), testee.key_comp()));
	}
	{
		const Set set = { "one", "two", "three", "four", "four", "three" };

		FlatSet testee(set.begin(), set.end());
		EXPECT_TRUE(std::is_sorted(testee.begin(), testee.end(), testee.key_comp()));
		EXPECT_TRUE(std::equal(set.begin(), set.end(), testee.begin()));
	}
}

TEST(FlatSetTest, Assignment)
{
	{
		FlatSet testee1 = GetSampleFlatSet();
		FlatSet testee2;
		FlatSet testee3;
		testee2 = testee1;
		testee3 = testee1;
		EXPECT_TRUE(std::equal(testee2.begin(), testee2.end(), testee3.begin()));
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
	EXPECT_TRUE(std::equal(sample.begin(), sample.end(), testee.begin()));

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
		FlatSet::iterator first = testee.find("Russians");
		ASSERT_TRUE(first != testee.end());
		testee.erase(first);
		ASSERT_FALSE(testee.count("Russians"));
	}
	{
		FlatSet testee(GetSampleFlatSet());
		FlatSet::iterator first = testee.find("Americans");
		FlatSet::iterator last = testee.find("Japaneses");
		ASSERT_TRUE(first != testee.end());
		ASSERT_TRUE(last != testee.end());
		testee.erase(first, last);
		ASSERT_FALSE(testee.erase("Americans"));
		ASSERT_TRUE(testee.count("Japaneses"));
	}
}

