#include <stingraykit/collection/flat_map.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	using Vector = std::vector<std::pair<std::string, std::string>>;
	using Map = std::map<std::string, std::string>;
	using FlatMap = flat_map<std::string, std::string>;

	template <class Collection, class Compare>
	bool IsSorted(const Collection& testee, const Compare& cmp)
	{
		for (typename Collection::const_iterator cur = testee.begin(), next = ++testee.begin(); next != testee.end(); ++cur, ++next)
			if (!cmp(*cur, *next))
				return false;
		return true;
	}

	Vector GetUnorderedVectorImpl()
	{
		Vector vec;
		vec.push_back(std::make_pair("one", "jaws"));
		vec.push_back(std::make_pair("two", "bite"));
		vec.push_back(std::make_pair("three", "claws"));
		vec.push_back(std::make_pair("four", "catch"));
		vec.push_back(std::make_pair("five", "blow"));
		vec.push_back(std::make_pair("six", "job"));
		vec.push_back(std::make_pair("seven", "two"));
		vec.push_back(std::make_pair("eight", "girls"));
		vec.push_back(std::make_pair("nine", "one"));
		vec.push_back(std::make_pair("ten", "cup"));
		return vec;
	}

	Vector GetUnorderedVector()
	{
		static Vector vec = GetUnorderedVectorImpl();
		return vec;
	}

	FlatMap GetSampleFlatMapImpl()
	{
		FlatMap map;
		map.insert(std::make_pair("Americans", "God blessed nation!"));
		map.insert(std::make_pair("Australians", "Got no idea why they don't fall!"));
		map.insert(std::make_pair("Japaneses", "Nation presented us anime!"));
		map.insert(std::make_pair("Russians", "Greatest nation in the world!"));
		map.insert(std::make_pair("Ukrainians", "Salo Ukraine! Geroyam Salo!"));
		return map;
	}

	FlatMap GetSampleFlatMap()
	{
		static FlatMap map = GetSampleFlatMapImpl();
		return map;
	}

	struct PairEquals
	{
		template < typename Lhs_, typename Rhs_ >
		bool operator () (const Lhs_& lhs, const Rhs_& rhs) const
		{ return lhs.first == rhs.first && lhs.second == rhs.second; }
	};

}


TEST(FlatMapTest, Construction)
{
	{
		FlatMap testee;
		EXPECT_TRUE(testee.empty());
	}
	{
		FlatMap testee((FlatMap::key_compare()));
		EXPECT_TRUE(testee.empty());
	}
	{
		FlatMap testee((FlatMap::key_compare()), (FlatMap::allocator_type()));
		EXPECT_TRUE(testee.empty());
	}
	{
		Vector vec;
		vec.push_back(std::make_pair("one", "jaws"));
		vec.push_back(std::make_pair("two", "bite"));
		vec.push_back(std::make_pair("three", "claws"));
		vec.push_back(std::make_pair("four", "catch"));
		vec.push_back(std::make_pair("four", "dup"));
		vec.push_back(std::make_pair("three", "dup"));

		FlatMap testee(vec.begin(), vec.end());
		EXPECT_EQ(testee.size(), (size_t)4);
		EXPECT_TRUE(IsSorted(testee, testee.value_comp()));
	}
	{
		Map map;
		map.insert(std::make_pair("one", "jaws"));
		map.insert(std::make_pair("two", "bite"));
		map.insert(std::make_pair("three", "claws"));
		map.insert(std::make_pair("four", "catch"));
		map.insert(std::make_pair("four", "dup"));
		map.insert(std::make_pair("three", "dup"));

		FlatMap testee(map.begin(), map.end());
		EXPECT_TRUE(IsSorted(testee, testee.value_comp()));
		EXPECT_TRUE(std::equal(map.begin(), map.end(), testee.begin(), PairEquals()));
	}
	{
		FlatMap testee;
		EXPECT_TRUE(testee.insert(std::make_pair("one", "jaws")).second);
		EXPECT_TRUE(testee.insert(std::make_pair("two", "bite")).second);
		EXPECT_TRUE(testee.insert(std::make_pair("three", "claws")).second);
		EXPECT_TRUE(testee.insert(std::make_pair("four", "catch")).second);
		EXPECT_FALSE(testee.insert(std::make_pair("four", "dup")).second);
		EXPECT_FALSE(testee.insert(std::make_pair("three", "dup")).second);

		EXPECT_TRUE(IsSorted(testee, testee.value_comp()));
	}
	{
		FlatMap testee1 = GetSampleFlatMap();
		FlatMap testee2;
		FlatMap testee3;
		testee2 = testee1;
		testee3 = testee1;
		EXPECT_TRUE(std::equal(testee2.begin(), testee2.end(), testee3.begin(), PairEquals()));
	}
}

TEST(FlatMapTest, Lookup)
{
	Vector unordered = GetUnorderedVector();

	FlatMap testee;
	Map sample;

	testee.reserve(unordered.size());
	for (Vector::const_iterator it = unordered.begin(); it != unordered.end(); ++it)
	{
		testee.insert(*it);
		sample.insert(*it);
	}

	EXPECT_TRUE(IsSorted(testee, testee.value_comp()));
	EXPECT_TRUE(std::equal(sample.begin(), sample.end(), testee.begin(), PairEquals()));

	for (Map::const_iterator sample_iter = sample.begin(); sample_iter != sample.end(); ++sample_iter)
	{
		FlatMap::const_iterator testee_iter = testee.find(sample_iter->first);
		ASSERT_NE(testee_iter, testee.end());
		EXPECT_TRUE(PairEquals()(*testee_iter, *sample_iter));
	}
	for (FlatMap::const_iterator testee_iter = testee.begin(); testee_iter != testee.end(); ++testee_iter)
	{
		Map::const_iterator sample_iter = sample.find(testee_iter->first);
		ASSERT_NE(sample_iter, sample.end());
		EXPECT_TRUE(PairEquals()(*sample_iter, *testee_iter));
	}

	for (Map::const_reverse_iterator sample_iter = sample.rbegin(); sample_iter != sample.rend(); ++sample_iter)
	{
		FlatMap::const_iterator testee_iter = testee.find(sample_iter->first);
		ASSERT_NE(testee_iter, testee.end());
		EXPECT_TRUE(PairEquals()(*testee_iter, *sample_iter));
	}

	for (FlatMap::const_reverse_iterator testee_iter = testee.rbegin(); testee_iter != testee.rend(); ++testee_iter)
	{
		Map::const_iterator sample_iter = sample.find(testee_iter->first);
		ASSERT_NE(sample_iter, sample.end());
		EXPECT_TRUE(PairEquals()(*sample_iter, *testee_iter));
	}

	for (Map::const_iterator sample_iter = sample.begin(); sample_iter != sample.end(); ++sample_iter)
		ASSERT_NO_THROW(EXPECT_EQ(testee.at(sample_iter->first), sample_iter->second));
	for (FlatMap::const_iterator testee_iter = testee.begin(); testee_iter != testee.end(); ++testee_iter)
		ASSERT_NO_THROW(EXPECT_EQ(sample.at(testee_iter->first), testee_iter->second));

	for (Map::const_iterator sample_iter = sample.begin(); sample_iter != sample.end(); ++sample_iter)
	{
		ASSERT_TRUE(testee.count(sample_iter->first));
		EXPECT_EQ(testee[sample_iter->first], sample_iter->second);
	}
	for (FlatMap::const_iterator testee_iter = testee.begin(); testee_iter != testee.end(); ++testee_iter)
	{
		ASSERT_TRUE(sample.count(testee_iter->first));
		EXPECT_EQ(sample[testee_iter->first], testee_iter->second);
	}
}

TEST(FlatMapTest, Removal)
{
	{
		FlatMap testee(GetSampleFlatMap());
		ASSERT_TRUE(testee.count("Ukrainians"));
		ASSERT_TRUE(testee.erase("Ukrainians"));
		ASSERT_FALSE(testee.erase("Ukrainians"));
	}
	{
		FlatMap testee(GetSampleFlatMap());
		FlatMap::iterator first = testee.find("Russians");
		ASSERT_TRUE(first != testee.end());
		testee.erase(first);
		ASSERT_FALSE(testee.count("Russians"));
	}
	{
		FlatMap testee(GetSampleFlatMap());
		FlatMap::iterator first = testee.find("Americans");
		FlatMap::iterator last = testee.find("Japaneses");
		ASSERT_TRUE(first != testee.end());
		ASSERT_TRUE(last != testee.end());
		testee.erase(first, last);
		ASSERT_FALSE(testee.erase("Americans"));
		ASSERT_TRUE(testee.count("Japaneses"));
	}
}
