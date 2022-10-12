#include <stingraykit/collection/ordered_map.h>

#include <gmock/gmock-matchers.h>

#include <map>

using namespace stingray;

using ::testing::ElementsAre;

namespace
{

	using Vector = std::vector<std::pair<std::string, std::string>>;
	using Map = std::map<std::string, std::string>;
	using OrderedMap = ordered_map<std::string, std::string>;

}


TEST(OrderedMapTest, Construction)
{
	{
		OrderedMap testee;
		ASSERT_TRUE(testee.empty());
	}
	{
		OrderedMap testee((OrderedMap::key_compare()));
		ASSERT_TRUE(testee.empty());
	}
	{
		OrderedMap testee((OrderedMap::allocator_type()));
		ASSERT_TRUE(testee.empty());
	}
	{
		const Vector vec = { { "2", "22" }, { "4", "44" }, { "3", "33" }, { "1", "11" }, { "1", "111" }, { "3", "333" } };

		OrderedMap testee1(vec.begin(), vec.end());
		OrderedMap testee2(vec.begin(), vec.end(), OrderedMap::allocator_type());

		ASSERT_EQ(testee1.size(), (size_t)4);
		ASSERT_EQ(testee2.size(), (size_t)4);

		ASSERT_TRUE(!std::is_sorted(testee1.begin(), testee1.end(), testee1.value_comp()));
		ASSERT_TRUE(!std::is_sorted(testee2.begin(), testee2.end(), testee2.value_comp()));

		ASSERT_THAT(testee1, ElementsAre(std::make_pair("2", "22"), std::make_pair("4", "44"), std::make_pair("3", "33"), std::make_pair("1", "11")));
		ASSERT_THAT(testee2, ElementsAre(std::make_pair("2", "22"), std::make_pair("4", "44"), std::make_pair("3", "33"), std::make_pair("1", "11")));
	}
	{
		const Map map = { { "2", "22" }, { "4", "44" }, { "3", "33" }, { "1", "11" }, { "1", "111" }, { "3", "333" } };

		OrderedMap testee(map.begin(), map.end());

		ASSERT_TRUE(std::is_sorted(testee.begin(), testee.end(), testee.value_comp()));
		ASSERT_TRUE(std::equal(map.begin(), map.end(), testee.begin(), testee.end()));

		ASSERT_THAT(testee, ElementsAre(std::make_pair("1", "11"), std::make_pair("2", "22"), std::make_pair("3", "33"), std::make_pair("4", "44")));
	}
	{
		OrderedMap testee1{ { "5", "55" }, { "1", "11" }, { "4", "44" }, { "2", "22" }, { "3", "33" } };

		OrderedMap testee2(testee1);
		ASSERT_FALSE(testee1.empty());

		OrderedMap testee3(testee1, OrderedMap::allocator_type());
		ASSERT_FALSE(testee1.empty());

		OrderedMap testee4(std::move(testee1));
		ASSERT_TRUE(testee1.empty());

		OrderedMap testee5(std::move(testee2), OrderedMap::allocator_type());
		ASSERT_TRUE(testee2.empty());

		ASSERT_TRUE(!std::is_sorted(testee3.begin(), testee3.end(), testee3.value_comp()));

		ASSERT_TRUE(std::equal(testee3.begin(), testee3.end(), testee4.begin(), testee4.end()));
		ASSERT_TRUE(std::equal(testee3.begin(), testee3.end(), testee5.begin(), testee5.end()));

		ASSERT_THAT(testee3, ElementsAre(std::make_pair("5", "55"), std::make_pair("1", "11"), std::make_pair("4", "44"), std::make_pair("2", "22"), std::make_pair("3", "33")));
	}
	{
		const std::initializer_list<OrderedMap::value_type> values{ { "2", "22" }, { "4", "44" }, { "3", "33" }, { "1", "11" }, { "1", "111" }, { "3", "333" } };

		OrderedMap testee1(values);
		OrderedMap testee2(values, OrderedMap::allocator_type());

		ASSERT_TRUE(!std::is_sorted(testee1.begin(), testee1.end(), testee1.value_comp()));

		ASSERT_TRUE(std::equal(testee1.begin(), testee1.end(), testee2.begin(), testee2.end()));

		ASSERT_THAT(testee1, ElementsAre(std::make_pair("2", "22"), std::make_pair("4", "44"), std::make_pair("3", "33"), std::make_pair("1", "11")));
	}
}

TEST(OrderedMapTest, Assignment)
{
	{
		const OrderedMap testee1{ { "5", "55" }, { "1", "11" }, { "4", "44" }, { "2", "22" }, { "3", "33" } };

		OrderedMap testee2;
		testee2 = testee1;

		ASSERT_TRUE(!std::is_sorted(testee2.begin(), testee2.end(), testee2.value_comp()));

		ASSERT_THAT(testee2, ElementsAre(std::make_pair("5", "55"), std::make_pair("1", "11"), std::make_pair("4", "44"), std::make_pair("2", "22"), std::make_pair("3", "33")));
	}
	{
		const std::initializer_list<OrderedMap::value_type> values{ { "2", "22" }, { "4", "44" }, { "3", "33" }, { "1", "11" }, { "1", "111" }, { "3", "333" } };

		OrderedMap testee;
		testee = values;

		ASSERT_TRUE(!std::is_sorted(testee.begin(), testee.end(), testee.value_comp()));

		ASSERT_THAT(testee, ElementsAre(std::make_pair("2", "22"), std::make_pair("4", "44"), std::make_pair("3", "33"), std::make_pair("1", "11")));
	}
}

TEST(OrderedMapTest, Brackets)
{
	{
		OrderedMap testee;

		OrderedMap::key_type two("2");
		OrderedMap::mapped_type twoTwo("22");
		OrderedMap::mapped_type& twoRef = testee[two];
		ASSERT_EQ(two, "2");
		ASSERT_TRUE(twoRef.empty());
		twoRef = twoTwo;
		ASSERT_EQ(twoTwo, "22");

		OrderedMap::key_type four("4");
		OrderedMap::mapped_type fourFour("44");
		OrderedMap::mapped_type& fourRef = testee[four];
		ASSERT_EQ(four, "4");
		ASSERT_TRUE(fourRef.empty());
		fourRef = fourFour;
		ASSERT_EQ(fourFour, "44");

		OrderedMap::key_type three("3");
		OrderedMap::mapped_type threeThree("33");
		OrderedMap::mapped_type& threeRef = testee[three];
		ASSERT_EQ(three, "3");
		ASSERT_TRUE(threeRef.empty());
		threeRef = threeThree;
		ASSERT_EQ(threeThree, "33");

		OrderedMap::key_type one("1");
		OrderedMap::mapped_type oneOne("11");
		OrderedMap::mapped_type& oneRef = testee[one];
		ASSERT_EQ(one, "1");
		ASSERT_TRUE(oneRef.empty());
		oneRef = oneOne;
		ASSERT_EQ(oneOne, "11");

		OrderedMap::key_type one2("1");
		OrderedMap::mapped_type oneOne2("111");
		OrderedMap::mapped_type& one2Ref = testee[one2];
		ASSERT_EQ(one2, "1");
		ASSERT_EQ(one2Ref, "11");
		one2Ref = oneOne2;
		ASSERT_EQ(oneOne2, "111");

		OrderedMap::key_type three2("3");
		OrderedMap::mapped_type threeThree2("333");
		OrderedMap::mapped_type& three2Ref = testee[three2];
		ASSERT_EQ(three2, "3");
		ASSERT_EQ(three2Ref, "33");
		three2Ref = threeThree2;
		ASSERT_EQ(threeThree2, "333");

		ASSERT_TRUE(!std::is_sorted(testee.begin(), testee.end(), testee.value_comp()));

		ASSERT_THAT(testee, ElementsAre(std::make_pair("2", "22"), std::make_pair("4", "44"), std::make_pair("3", "333"), std::make_pair("1", "111")));
	}
	{
		OrderedMap testee;

		OrderedMap::key_type two("2");
		OrderedMap::mapped_type twoTwo("22");
		OrderedMap::mapped_type& twoRef = testee[std::move(two)];
		ASSERT_TRUE(two.empty());
		ASSERT_TRUE(twoRef.empty());
		twoRef = std::move(twoTwo);
		ASSERT_TRUE(twoTwo.empty());

		OrderedMap::key_type four("4");
		OrderedMap::mapped_type fourFour("44");
		OrderedMap::mapped_type& fourRef = testee[std::move(four)];
		ASSERT_TRUE(four.empty());
		ASSERT_TRUE(fourRef.empty());
		fourRef = std::move(fourFour);
		ASSERT_TRUE(fourFour.empty());

		OrderedMap::key_type three("3");
		OrderedMap::mapped_type threeThree("33");
		OrderedMap::mapped_type& threeRef = testee[std::move(three)];
		ASSERT_TRUE(three.empty());
		ASSERT_TRUE(threeRef.empty());
		threeRef = std::move(threeThree);
		ASSERT_TRUE(threeThree.empty());

		OrderedMap::key_type one("1");
		OrderedMap::mapped_type oneOne("11");
		OrderedMap::mapped_type& oneRef = testee[std::move(one)];
		ASSERT_TRUE(one.empty());
		ASSERT_TRUE(oneRef.empty());
		oneRef = std::move(oneOne);
		ASSERT_TRUE(oneOne.empty());

		OrderedMap::key_type one2("1");
		OrderedMap::mapped_type oneOne2("111");
		OrderedMap::mapped_type& one2Ref = testee[std::move(one2)];
		ASSERT_EQ(one2, "1");
		ASSERT_EQ(one2Ref, "11");
		one2Ref = std::move(oneOne2);
		ASSERT_TRUE(oneOne2.empty());

		OrderedMap::key_type three2("3");
		OrderedMap::mapped_type threeThree2("333");
		OrderedMap::mapped_type& three2Ref = testee[std::move(three2)];
		ASSERT_EQ(three2, "3");
		ASSERT_EQ(three2Ref, "33");
		three2Ref = std::move(threeThree2);
		ASSERT_TRUE(threeThree2.empty());

		ASSERT_TRUE(!std::is_sorted(testee.begin(), testee.end(), testee.value_comp()));

		ASSERT_THAT(testee, ElementsAre(std::make_pair("2", "22"), std::make_pair("4", "44"), std::make_pair("3", "333"), std::make_pair("1", "111")));
	}
}

TEST(OrderedMapTest, Insertion)
{
	{
		OrderedMap testee;

		OrderedMap::value_type two("2", "22");
		auto twoResult = testee.insert(two);
		ASSERT_EQ(two.first, "2");
		ASSERT_EQ(two.second, "22");
		ASSERT_TRUE(twoResult.second);
		ASSERT_EQ(twoResult.first->first, "2");
		ASSERT_EQ(twoResult.first->second, "22");

		OrderedMap::value_type four("4", "44");
		auto fourResult = testee.insert(four);
		ASSERT_EQ(four.first, "4");
		ASSERT_EQ(four.second, "44");
		ASSERT_TRUE(fourResult.second);
		ASSERT_EQ(fourResult.first->first, "4");
		ASSERT_EQ(fourResult.first->second, "44");

		OrderedMap::value_type three("3", "33");
		auto threeResult = testee.insert(three);
		ASSERT_EQ(three.first, "3");
		ASSERT_EQ(three.second, "33");
		ASSERT_TRUE(threeResult.second);
		ASSERT_EQ(threeResult.first->first, "3");
		ASSERT_EQ(threeResult.first->second, "33");

		OrderedMap::value_type one("1", "11");
		auto oneResult = testee.insert(one);
		ASSERT_EQ(one.first, "1");
		ASSERT_EQ(one.second, "11");
		ASSERT_TRUE(oneResult.second);
		ASSERT_EQ(oneResult.first->first, "1");
		ASSERT_EQ(oneResult.first->second, "11");

		OrderedMap::value_type one2("1", "111");
		auto one2Result = testee.insert(one2);
		ASSERT_EQ(one2.first, "1");
		ASSERT_EQ(one2.second, "111");
		ASSERT_FALSE(one2Result.second);
		ASSERT_EQ(one2Result.first->first, "1");
		ASSERT_EQ(one2Result.first->second, "11");

		OrderedMap::value_type three2("3", "333");
		auto three2Result = testee.insert(three2);
		ASSERT_EQ(three2.first, "3");
		ASSERT_EQ(three2.second, "333");
		ASSERT_FALSE(three2Result.second);
		ASSERT_EQ(three2Result.first->first, "3");
		ASSERT_EQ(three2Result.first->second, "33");

		ASSERT_TRUE(!std::is_sorted(testee.begin(), testee.end(), testee.value_comp()));

		ASSERT_THAT(testee, ElementsAre(std::make_pair("2", "22"), std::make_pair("4", "44"), std::make_pair("3", "33"), std::make_pair("1", "11")));
	}
	{
		OrderedMap testee;

		OrderedMap::value_type two("2", "22");
		auto twoResult = testee.insert(std::move(two));
		ASSERT_EQ(two.first, "2");
		ASSERT_TRUE(two.second.empty());
		ASSERT_TRUE(twoResult.second);
		ASSERT_EQ(twoResult.first->first, "2");
		ASSERT_EQ(twoResult.first->second, "22");

		OrderedMap::value_type four("4", "44");
		auto fourResult = testee.insert(std::move(four));
		ASSERT_EQ(four.first, "4");
		ASSERT_TRUE(four.second.empty());
		ASSERT_TRUE(fourResult.second);
		ASSERT_EQ(fourResult.first->first, "4");
		ASSERT_EQ(fourResult.first->second, "44");

		OrderedMap::value_type three("3", "33");
		auto threeResult = testee.insert(std::move(three));
		ASSERT_EQ(three.first, "3");
		ASSERT_TRUE(three.second.empty());
		ASSERT_TRUE(threeResult.second);
		ASSERT_EQ(threeResult.first->first, "3");
		ASSERT_EQ(threeResult.first->second, "33");

		OrderedMap::value_type one("1", "11");
		auto oneResult = testee.insert(std::move(one));
		ASSERT_EQ(one.first, "1");
		ASSERT_TRUE(one.second.empty());
		ASSERT_TRUE(oneResult.second);
		ASSERT_EQ(oneResult.first->first, "1");
		ASSERT_EQ(oneResult.first->second, "11");

		OrderedMap::value_type one2("1", "111");
		auto one2Result = testee.insert(std::move(one2));
		ASSERT_EQ(one2.first, "1");
		ASSERT_EQ(one2.second, "111");
		ASSERT_FALSE(one2Result.second);
		ASSERT_EQ(one2Result.first->first, "1");
		ASSERT_EQ(one2Result.first->second, "11");

		OrderedMap::value_type three2("3", "333");
		auto three2Result = testee.insert(std::move(three2));
		ASSERT_EQ(three2.first, "3");
		ASSERT_EQ(three2.second, "333");
		ASSERT_FALSE(three2Result.second);
		ASSERT_EQ(three2Result.first->first, "3");
		ASSERT_EQ(three2Result.first->second, "33");

		ASSERT_TRUE(!std::is_sorted(testee.begin(), testee.end(), testee.value_comp()));

		ASSERT_THAT(testee, ElementsAre(std::make_pair("2", "22"), std::make_pair("4", "44"), std::make_pair("3", "33"), std::make_pair("1", "11")));
	}
	{
		OrderedMap testee;

		OrderedMap::value_type two("2", "22");
		auto twoIt = testee.insert(testee.end(), two);
		ASSERT_EQ(two.first, "2");
		ASSERT_EQ(two.second, "22");
		ASSERT_EQ(twoIt->first, "2");
		ASSERT_EQ(twoIt->second, "22");

		OrderedMap::value_type four("4", "44");
		auto fourIt = testee.insert(testee.end(), four);
		ASSERT_EQ(four.first, "4");
		ASSERT_EQ(four.second, "44");
		ASSERT_EQ(fourIt->first, "4");
		ASSERT_EQ(fourIt->second, "44");

		OrderedMap::value_type three("3", "33");
		auto threeIt = testee.insert(testee.begin(), three);
		ASSERT_EQ(three.first, "3");
		ASSERT_EQ(three.second, "33");
		ASSERT_EQ(threeIt->first, "3");
		ASSERT_EQ(threeIt->second, "33");

		OrderedMap::value_type one("1", "11");
		auto oneIt = testee.insert(testee.begin(), one);
		ASSERT_EQ(one.first, "1");
		ASSERT_EQ(one.second, "11");
		ASSERT_EQ(oneIt->first, "1");
		ASSERT_EQ(oneIt->second, "11");

		OrderedMap::value_type one2("1", "111");
		auto one2It = testee.insert(testee.end(), one2);
		ASSERT_EQ(one2.first, "1");
		ASSERT_EQ(one2.second, "111");
		ASSERT_EQ(one2It->first, "1");
		ASSERT_EQ(one2It->second, "11");

		OrderedMap::value_type three2("3", "333");
		auto three2It = testee.insert(testee.end(), three2);
		ASSERT_EQ(three2.first, "3");
		ASSERT_EQ(three2.second, "333");
		ASSERT_EQ(three2It->first, "3");
		ASSERT_EQ(three2It->second, "33");

		OrderedMap::const_iterator twoIt2 = testee.find("2");
		ASSERT_EQ(twoIt2->first, "2");
		ASSERT_EQ(twoIt2->second, "22");
		OrderedMap::value_type five("5", "55");
		auto fiveIt = testee.insert(twoIt2, five);
		ASSERT_EQ(five.first, "5");
		ASSERT_EQ(five.second, "55");
		ASSERT_EQ(fiveIt->first, "5");
		ASSERT_EQ(fiveIt->second, "55");

		ASSERT_TRUE(!std::is_sorted(testee.begin(), testee.end(), testee.value_comp()));

		ASSERT_THAT(testee, ElementsAre(std::make_pair("1", "11"), std::make_pair("3", "33"), std::make_pair("5", "55"), std::make_pair("2", "22"), std::make_pair("4", "44")));
	}
	{
		OrderedMap testee;

		OrderedMap::value_type two("2", "22");
		auto twoIt = testee.insert(testee.end(), std::move(two));
		ASSERT_EQ(two.first, "2");
		ASSERT_TRUE(two.second.empty());
		ASSERT_EQ(twoIt->first, "2");
		ASSERT_EQ(twoIt->second, "22");

		OrderedMap::value_type four("4", "44");
		auto fourIt = testee.insert(testee.end(), std::move(four));
		ASSERT_EQ(four.first, "4");
		ASSERT_TRUE(four.second.empty());
		ASSERT_EQ(fourIt->first, "4");
		ASSERT_EQ(fourIt->second, "44");

		OrderedMap::value_type three("3", "33");
		auto threeIt = testee.insert(testee.begin(), std::move(three));
		ASSERT_EQ(three.first, "3");
		ASSERT_TRUE(three.second.empty());
		ASSERT_EQ(threeIt->first, "3");
		ASSERT_EQ(threeIt->second, "33");

		OrderedMap::value_type one("1", "11");
		auto oneIt = testee.insert(testee.begin(), std::move(one));
		ASSERT_EQ(one.first, "1");
		ASSERT_TRUE(one.second.empty());
		ASSERT_EQ(oneIt->first, "1");
		ASSERT_EQ(oneIt->second, "11");

		OrderedMap::value_type one2("1", "111");
		auto one2It = testee.insert(testee.end(), std::move(one2));
		ASSERT_EQ(one2.first, "1");
		ASSERT_EQ(one2.second, "111");
		ASSERT_EQ(one2It->first, "1");
		ASSERT_EQ(one2It->second, "11");

		OrderedMap::value_type three2("3", "333");
		auto three2It = testee.insert(testee.end(), std::move(three2));
		ASSERT_EQ(three2.first, "3");
		ASSERT_EQ(three2.second, "333");
		ASSERT_EQ(three2It->first, "3");
		ASSERT_EQ(three2It->second, "33");

		OrderedMap::const_iterator twoIt2 = testee.find("2");
		ASSERT_EQ(twoIt2->first, "2");
		ASSERT_EQ(twoIt2->second, "22");
		OrderedMap::value_type five("5", "55");
		auto fiveIt = testee.insert(twoIt2, std::move(five));
		ASSERT_EQ(five.first, "5");
		ASSERT_TRUE(five.second.empty());
		ASSERT_EQ(fiveIt->first, "5");
		ASSERT_EQ(fiveIt->second, "55");

		ASSERT_TRUE(!std::is_sorted(testee.begin(), testee.end(), testee.value_comp()));

		ASSERT_THAT(testee, ElementsAre(std::make_pair("1", "11"), std::make_pair("3", "33"), std::make_pair("5", "55"), std::make_pair("2", "22"), std::make_pair("4", "44")));
	}
}

TEST(OrderedMapTest, Emplacing)
{
	{
		OrderedMap testee;

		OrderedMap::key_type twoFirst("2");
		OrderedMap::mapped_type twoSecond("22");
		auto twoResult = testee.emplace(std::move(twoFirst), std::move(twoSecond));
		ASSERT_TRUE(twoFirst.empty());
		ASSERT_TRUE(twoSecond.empty());
		ASSERT_TRUE(twoResult.second);
		ASSERT_EQ(twoResult.first->first, "2");
		ASSERT_EQ(twoResult.first->second, "22");

		OrderedMap::key_type fourFirst("4");
		OrderedMap::mapped_type fourSecond("44");
		auto fourResult = testee.emplace(std::move(fourFirst), std::move(fourSecond));
		ASSERT_TRUE(fourFirst.empty());
		ASSERT_TRUE(fourSecond.empty());
		ASSERT_TRUE(fourResult.second);
		ASSERT_EQ(fourResult.first->first, "4");
		ASSERT_EQ(fourResult.first->second, "44");

		OrderedMap::key_type threeFirst("3");
		OrderedMap::mapped_type threeSecond("33");
		auto threeResult = testee.emplace(std::move(threeFirst), std::move(threeSecond));
		ASSERT_TRUE(threeFirst.empty());
		ASSERT_TRUE(threeSecond.empty());
		ASSERT_TRUE(threeResult.second);
		ASSERT_EQ(threeResult.first->first, "3");
		ASSERT_EQ(threeResult.first->second, "33");

		OrderedMap::key_type oneFirst("1");
		OrderedMap::mapped_type oneSecond("11");
		auto oneResult = testee.emplace(std::move(oneFirst), std::move(oneSecond));
		ASSERT_TRUE(oneFirst.empty());
		ASSERT_TRUE(oneSecond.empty());
		ASSERT_TRUE(oneResult.second);
		ASSERT_EQ(oneResult.first->first, "1");
		ASSERT_EQ(oneResult.first->second, "11");

		OrderedMap::key_type one2First("1");
		OrderedMap::mapped_type one2Second("111");
		auto one2Result = testee.emplace(std::move(one2First), std::move(one2Second));
		ASSERT_TRUE(one2First.empty());
		ASSERT_TRUE(one2Second.empty());
		ASSERT_FALSE(one2Result.second);
		ASSERT_EQ(one2Result.first->first, "1");
		ASSERT_EQ(one2Result.first->second, "11");

		OrderedMap::key_type three2First("3");
		OrderedMap::mapped_type three2Second("333");
		auto three2Result = testee.emplace(std::move(three2First), std::move(three2Second));
		ASSERT_TRUE(three2First.empty());
		ASSERT_TRUE(three2Second.empty());
		ASSERT_FALSE(three2Result.second);
		ASSERT_EQ(three2Result.first->first, "3");
		ASSERT_EQ(three2Result.first->second, "33");

		ASSERT_TRUE(!std::is_sorted(testee.begin(), testee.end(), testee.value_comp()));

		ASSERT_THAT(testee, ElementsAre(std::make_pair("2", "22"), std::make_pair("4", "44"), std::make_pair("3", "33"), std::make_pair("1", "11")));
	}
	{
		OrderedMap testee;

		OrderedMap::key_type twoFirst("2");
		OrderedMap::mapped_type twoSecond("22");
		auto twoIt = testee.emplace_hint(testee.end(), std::move(twoFirst), std::move(twoSecond));
		ASSERT_TRUE(twoFirst.empty());
		ASSERT_TRUE(twoSecond.empty());
		ASSERT_EQ(twoIt->first, "2");
		ASSERT_EQ(twoIt->second, "22");

		OrderedMap::key_type fourFirst("4");
		OrderedMap::mapped_type fourSecond("44");
		auto fourIt = testee.emplace_hint(testee.end(), std::move(fourFirst), std::move(fourSecond));
		ASSERT_TRUE(fourFirst.empty());
		ASSERT_TRUE(fourSecond.empty());
		ASSERT_EQ(fourIt->first, "4");
		ASSERT_EQ(fourIt->second, "44");

		OrderedMap::key_type threeFirst("3");
		OrderedMap::mapped_type threeSecond("33");
		auto threeIt = testee.emplace_hint(testee.begin(), std::move(threeFirst), std::move(threeSecond));
		ASSERT_TRUE(threeFirst.empty());
		ASSERT_TRUE(threeSecond.empty());
		ASSERT_EQ(threeIt->first, "3");
		ASSERT_EQ(threeIt->second, "33");

		OrderedMap::key_type oneFirst("1");
		OrderedMap::mapped_type oneSecond("11");
		auto oneIt = testee.emplace_hint(testee.begin(), std::move(oneFirst), std::move(oneSecond));
		ASSERT_TRUE(oneFirst.empty());
		ASSERT_TRUE(oneSecond.empty());
		ASSERT_EQ(oneIt->first, "1");
		ASSERT_EQ(oneIt->second, "11");

		OrderedMap::key_type one2First("1");
		OrderedMap::mapped_type one2Second("111");
		auto one2It = testee.emplace_hint(testee.end(), std::move(one2First), std::move(one2Second));
		ASSERT_TRUE(one2First.empty());
		ASSERT_TRUE(one2Second.empty());
		ASSERT_EQ(one2It->first, "1");
		ASSERT_EQ(one2It->second, "11");

		OrderedMap::key_type three2First("3");
		OrderedMap::mapped_type three2Second("333");
		auto three2It = testee.emplace_hint(testee.end(), std::move(three2First), std::move(three2Second));
		ASSERT_TRUE(three2First.empty());
		ASSERT_TRUE(three2Second.empty());
		ASSERT_EQ(three2It->first, "3");
		ASSERT_EQ(three2It->second, "33");

		OrderedMap::const_iterator twoIt2 = testee.find("2");
		ASSERT_EQ(twoIt2->first, "2");
		ASSERT_EQ(twoIt2->second, "22");
		OrderedMap::key_type fiveFirst("5");
		OrderedMap::mapped_type fiveSecond("55");
		auto fiveIt = testee.emplace_hint(twoIt2, std::move(fiveFirst), std::move(fiveSecond));
		ASSERT_TRUE(fiveFirst.empty());
		ASSERT_TRUE(fiveSecond.empty());
		ASSERT_EQ(fiveIt->first, "5");
		ASSERT_EQ(fiveIt->second, "55");

		ASSERT_TRUE(!std::is_sorted(testee.begin(), testee.end(), testee.value_comp()));

		ASSERT_THAT(testee, ElementsAre(std::make_pair("1", "11"), std::make_pair("3", "33"), std::make_pair("5", "55"), std::make_pair("2", "22"), std::make_pair("4", "44")));
	}

	{
		OrderedMap testee;

		auto twoResult = testee.emplace(std::piecewise_construct, std::make_tuple(1, '2'), std::make_tuple(2, '2'));
		ASSERT_TRUE(twoResult.second);
		ASSERT_EQ(twoResult.first->first, "2");
		ASSERT_EQ(twoResult.first->second, "22");

		auto emptyResult = testee.emplace(std::piecewise_construct, std::make_tuple(), std::make_tuple());
		ASSERT_TRUE(emptyResult.second);
		ASSERT_EQ(emptyResult.first->first, "");
		ASSERT_EQ(emptyResult.first->second, "");

		ASSERT_TRUE(!std::is_sorted(testee.begin(), testee.end(), testee.value_comp()));

		ASSERT_THAT(testee, ElementsAre(std::make_pair("2", "22"), std::make_pair("", "")));
	}
}

TEST(OrderedMapTest, Lookup)
{
	const Vector unsorted{ { "5", "55" }, { "4", "44" }, { "8", "88" }, { "9", "99" }, { "1", "11" }, { "6", "66" }, { "3", "33" }, { "2", "22" }, { "7", "77" }, { "0", "00" } };

	{
		OrderedMap testee(unsorted.begin(), unsorted.end());
		Map sample(unsorted.begin(), unsorted.end());

		ASSERT_TRUE(!std::is_sorted(testee.begin(), testee.end(), testee.value_comp()));

		for (Map::iterator sampleIt = sample.begin(); sampleIt != sample.end(); ++sampleIt)
		{
			OrderedMap::iterator testeeIt = testee.find(sampleIt->first);
			ASSERT_NE(testeeIt, testee.end());
			ASSERT_TRUE(*testeeIt == *sampleIt);
		}

		for (OrderedMap::iterator testeeIt = testee.begin(); testeeIt != testee.end(); ++testeeIt)
		{
			Map::iterator sampleIt = sample.find(testeeIt->first);
			ASSERT_NE(sampleIt, sample.end());
			ASSERT_TRUE(*sampleIt == *testeeIt);
		}

		for (Map::reverse_iterator sampleIt = sample.rbegin(); sampleIt != sample.rend(); ++sampleIt)
		{
			OrderedMap::iterator testeeIt = testee.find(sampleIt->first);
			ASSERT_NE(testeeIt, testee.end());
			ASSERT_TRUE(*testeeIt == *sampleIt);
		}

		for (OrderedMap::reverse_iterator testeeIt = testee.rbegin(); testeeIt != testee.rend(); ++testeeIt)
		{
			Map::iterator sampleIt = sample.find(testeeIt->first);
			ASSERT_NE(sampleIt, sample.end());
			ASSERT_TRUE(*sampleIt == *testeeIt);
		}

		for (Map::iterator sampleIt = sample.begin(); sampleIt != sample.end(); ++sampleIt)
		{
			ASSERT_NO_THROW(EXPECT_EQ(testee.at(sampleIt->first), sampleIt->second));
			ASSERT_THROW(testee.at(sampleIt->second), KeyNotFoundException);
		}

		for (OrderedMap::iterator testeeIt = testee.begin(); testeeIt != testee.end(); ++testeeIt)
		{
			ASSERT_NO_THROW(EXPECT_EQ(sample.at(testeeIt->first), testeeIt->second));
			ASSERT_ANY_THROW(testee.at(testeeIt->second));
		}

		for (Map::iterator sampleIt = sample.begin(); sampleIt != sample.end(); ++sampleIt)
			ASSERT_EQ(testee.count(sampleIt->first), 1);

		for (OrderedMap::iterator testeeIt = testee.begin(); testeeIt != testee.end(); ++testeeIt)
			ASSERT_EQ(sample.count(testeeIt->first), 1);
	}

	{
		const OrderedMap testee(unsorted.begin(), unsorted.end());
		const Map sample(unsorted.begin(), unsorted.end());

		ASSERT_TRUE(!std::is_sorted(testee.begin(), testee.end(), testee.value_comp()));

		for (Map::const_iterator sampleIt = sample.begin(); sampleIt != sample.end(); ++sampleIt)
		{
			OrderedMap::const_iterator testeeIt = testee.find(sampleIt->first);
			ASSERT_NE(testeeIt, testee.end());
			ASSERT_TRUE(*testeeIt == *sampleIt);
		}

		for (OrderedMap::const_iterator testeeIt = testee.begin(); testeeIt != testee.end(); ++testeeIt)
		{
			Map::const_iterator sampleIt = sample.find(testeeIt->first);
			ASSERT_NE(sampleIt, sample.end());
			ASSERT_TRUE(*sampleIt == *testeeIt);
		}

		for (Map::const_reverse_iterator sampleIt = sample.rbegin(); sampleIt != sample.rend(); ++sampleIt)
		{
			OrderedMap::const_iterator testeeIt = testee.find(sampleIt->first);
			ASSERT_NE(testeeIt, testee.end());
			ASSERT_TRUE(*testeeIt == *sampleIt);
		}

		for (OrderedMap::const_reverse_iterator testeeIt = testee.rbegin(); testeeIt != testee.rend(); ++testeeIt)
		{
			Map::const_iterator sampleIt = sample.find(testeeIt->first);
			ASSERT_NE(sampleIt, sample.end());
			ASSERT_TRUE(*sampleIt == *testeeIt);
		}

		for (Map::const_iterator sampleIt = sample.begin(); sampleIt != sample.end(); ++sampleIt)
		{
			ASSERT_NO_THROW(EXPECT_EQ(testee.at(sampleIt->first), sampleIt->second));
			ASSERT_THROW(testee.at(sampleIt->second), KeyNotFoundException);
		}

		for (OrderedMap::const_iterator testeeIt = testee.begin(); testeeIt != testee.end(); ++testeeIt)
		{
			ASSERT_NO_THROW(EXPECT_EQ(sample.at(testeeIt->first), testeeIt->second));
			ASSERT_ANY_THROW(testee.at(testeeIt->second));
		}

		for (Map::const_iterator sampleIt = sample.begin(); sampleIt != sample.end(); ++sampleIt)
			ASSERT_EQ(testee.count(sampleIt->first), 1);

		for (OrderedMap::const_iterator testeeIt = testee.begin(); testeeIt != testee.end(); ++testeeIt)
			ASSERT_EQ(sample.count(testeeIt->first), 1);
	}
}

TEST(OrderedMapTest, Swap)
{
	{
		OrderedMap testee1{ { "5", "55" }, { "1", "11" }, { "4", "44" }, { "2", "22" }, { "3", "33" } };
		OrderedMap testee2;

		testee1.swap(testee2);

		ASSERT_TRUE(testee1.empty());
		ASSERT_THAT(testee2, ElementsAre(std::make_pair("5", "55"), std::make_pair("1", "11"), std::make_pair("4", "44"), std::make_pair("2", "22"), std::make_pair("3", "33")));
	}
}

TEST(OrderedMapTest, Removal)
{
	{
		OrderedMap testee{ { "5", "55" }, { "1", "11" }, { "4", "44" }, { "2", "22" }, { "3", "33" } };

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

		ASSERT_THAT(testee, ElementsAre(std::make_pair("1", "11"), std::make_pair("2", "22")));
	}
	{
		OrderedMap testee{ { "5", "55" }, { "1", "11" }, { "4", "44" }, { "2", "22" }, { "3", "33" } };

		OrderedMap::const_iterator fourIt = testee.find("4");
		ASSERT_NE(fourIt, testee.end());
		ASSERT_EQ(fourIt->first, "4");
		ASSERT_EQ(fourIt->second, "44");
		OrderedMap::const_iterator twoIt = testee.erase(fourIt);
		ASSERT_NE(twoIt, testee.end());
		ASSERT_EQ(twoIt->first, "2");
		ASSERT_EQ(twoIt->second, "22");
		ASSERT_FALSE(testee.count("4"));

		OrderedMap::const_iterator fiveIt = testee.find("5");
		ASSERT_NE(fiveIt, testee.end());
		ASSERT_EQ(fiveIt->first, "5");
		ASSERT_EQ(fiveIt->second, "55");
		OrderedMap::const_iterator oneIt = testee.erase(fiveIt);
		ASSERT_NE(oneIt, testee.end());
		ASSERT_EQ(oneIt->first, "1");
		ASSERT_EQ(oneIt->second, "11");
		ASSERT_FALSE(testee.count("5"));

		OrderedMap::const_iterator threeIt = testee.find("3");
		ASSERT_NE(threeIt, testee.end());
		ASSERT_EQ(threeIt->first, "3");
		ASSERT_EQ(threeIt->second, "33");
		OrderedMap::const_iterator endIt = testee.erase(threeIt);
		ASSERT_EQ(endIt, testee.end());
		ASSERT_FALSE(testee.count("5"));

		ASSERT_THAT(testee, ElementsAre(std::make_pair("1", "11"), std::make_pair("2", "22")));
	}
	{
		OrderedMap testee{ { "5", "55" }, { "1", "11" }, { "4", "44" }, { "2", "22" }, { "3", "33" } };

		OrderedMap::const_iterator oneIt = testee.find("1");
		ASSERT_NE(oneIt, testee.end());
		ASSERT_EQ(oneIt->first, "1");
		ASSERT_EQ(oneIt->second, "11");
		OrderedMap::const_iterator twoIt = testee.find("2");
		ASSERT_NE(twoIt, testee.end());
		ASSERT_EQ(twoIt->first, "2");
		ASSERT_EQ(twoIt->second, "22");
		OrderedMap::const_iterator two2It = testee.erase(oneIt, twoIt);
		ASSERT_NE(two2It, testee.end());
		ASSERT_EQ(two2It->first, "2");
		ASSERT_EQ(two2It->second, "22");
		ASSERT_FALSE(testee.count("1"));
		ASSERT_FALSE(testee.count("4"));
		ASSERT_TRUE(testee.count("2"));

		ASSERT_THAT(testee, ElementsAre(std::make_pair("5", "55"), std::make_pair("2", "22"), std::make_pair("3", "33")));

		OrderedMap::const_iterator endIt = testee.erase(testee.begin(), testee.end());
		ASSERT_EQ(endIt, testee.end());

		ASSERT_TRUE(testee.empty());
	}
}

TEST(OrderedMapTest, Comparison)
{
	{
		const OrderedMap testee1{ { "5", "55" }, { "1", "11" }, { "4", "44" }, { "2", "22" }, { "3", "33" } };
		const OrderedMap testee2(testee1);

		ASSERT_TRUE(testee1 == testee2);
		ASSERT_FALSE(testee1 != testee2);
		ASSERT_FALSE(testee1 < testee2);
		ASSERT_FALSE(testee1 > testee2);
		ASSERT_TRUE(testee1 <= testee2);
		ASSERT_TRUE(testee1 >= testee2);
	}
	{
		const OrderedMap testee1{ { "5", "55" }, { "1", "11" }, { "4", "44" }, { "2", "22" }, { "3", "33" } };
		const OrderedMap testee2{ { "5", "55" }, { "1", "11" }, { "4", "44" }, { "2", "22" } };

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
		const OrderedMap testee1{ { "5", "55" }, { "4", "44" }, { "1", "11" }, { "2", "22" }, { "3", "33" } };
		const OrderedMap testee2{ { "5", "55" }, { "1", "11" }, { "4", "44" }, { "2", "22" }, { "3", "33" } };

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
