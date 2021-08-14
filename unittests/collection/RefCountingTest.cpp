#include <stingraykit/collection/ForEach.h>
#include <stingraykit/collection/RefCounter.h>
#include <stingraykit/collection/RefCountingMap.h>
#include <stingraykit/collection/RefCountingSet.h>
#include <stingraykit/collection/RefCountingValue.h>
#include <stingraykit/function/bind.h>
#include <stingraykit/function/functional.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	struct ItemsHolder
	{
		std::multiset<int> Items;

		EmptyType Add(int key)
		{ Items.insert(key); return EmptyType(); }

		void Remove(int key)
		{ Items.erase(key); }
	};

}


TEST(RefCountingTest, Map)
{
	typedef RefCountingMap<int, EmptyType> Map;

	ItemsHolder ih;
	Map rm;

	ASSERT_TRUE(ih.Items.empty());
	ASSERT_TRUE(rm.empty());
	ASSERT_TRUE(rm.begin() == rm.end());

	rm.add(1, Bind(&ItemsHolder::Add, &ih, _1));

	ASSERT_TRUE(!rm.empty());
	ASSERT_EQ(rm.size(), 1u);
	ASSERT_EQ(ih.Items.size(), 1u);
	{
		int seq[] = {1};
		ASSERT_TRUE(std::equal(ih.Items.begin(), ih.Items.end(), std::begin(seq), std::end(seq)));
		ASSERT_TRUE(std::equal(keys_iterator(rm.begin()), keys_iterator(rm.end()), std::begin(seq), std::end(seq)));
	}

	rm.add(42, Bind(&ItemsHolder::Add, &ih, _1));

	ASSERT_TRUE(!rm.empty());
	ASSERT_EQ(rm.size(), 2u);
	ASSERT_EQ(ih.Items.size(), 2u);
	{
		int seq[] = {1, 42};
		ASSERT_TRUE(std::equal(ih.Items.begin(), ih.Items.end(), std::begin(seq), std::end(seq)));
		ASSERT_TRUE(std::equal(keys_iterator(rm.begin()), keys_iterator(rm.end()), std::begin(seq), std::end(seq)));
	}

	rm.add(42, Bind(&ItemsHolder::Add, &ih, _1));

	ASSERT_TRUE(!rm.empty());
	ASSERT_EQ(rm.size(), 2u);
	ASSERT_EQ(ih.Items.size(), 2u);
	{
		int seq[] = {1, 42};
		ASSERT_TRUE(std::equal(ih.Items.begin(), ih.Items.end(), std::begin(seq), std::end(seq)));
		ASSERT_TRUE(std::equal(keys_iterator(rm.begin()), keys_iterator(rm.end()), std::begin(seq), std::end(seq)));
	}

	Map::iterator it = rm.find(42);
	ASSERT_TRUE(it != rm.end());
	rm.erase(it, Bind(&ItemsHolder::Remove, &ih, _1));

	ASSERT_TRUE(!rm.empty());
	ASSERT_EQ(rm.size(), 2u);
	ASSERT_EQ(ih.Items.size(), 2u);
	{
		int seq[] = {1, 42};
		ASSERT_TRUE(std::equal(ih.Items.begin(), ih.Items.end(), std::begin(seq), std::end(seq)));
		ASSERT_TRUE(std::equal(keys_iterator(rm.begin()), keys_iterator(rm.end()), std::begin(seq), std::end(seq)));
	}
}


TEST(RefCountingTest, Set)
{
	typedef RefCountingSet<int> Set;

	ItemsHolder ih;
	Set rs;

	ASSERT_TRUE(ih.Items.empty());
	ASSERT_TRUE(rs.empty());
	ASSERT_TRUE(rs.begin() == rs.end());

	rs.add(1, Bind(&ItemsHolder::Add, &ih, _1));

	ASSERT_TRUE(!rs.empty());
	ASSERT_EQ(rs.size(), 1u);
	ASSERT_EQ(ih.Items.size(), 1u);
	{
		int seq[] = {1};
		ASSERT_TRUE(std::equal(ih.Items.begin(), ih.Items.end(), std::begin(seq), std::end(seq)));
		ASSERT_TRUE(std::equal(rs.begin(), rs.end(), std::begin(seq), std::end(seq)));
	}

	rs.add(42, Bind(&ItemsHolder::Add, &ih, _1));

	ASSERT_TRUE(!rs.empty());
	ASSERT_EQ(rs.size(), 2u);
	ASSERT_EQ(ih.Items.size(), 2u);
	{
		int seq[] = {1, 42};
		ASSERT_TRUE(std::equal(ih.Items.begin(), ih.Items.end(), std::begin(seq), std::end(seq)));
		ASSERT_TRUE(std::equal(rs.begin(), rs.end(), std::begin(seq), std::end(seq)));
	}

	rs.add(42, Bind(&ItemsHolder::Add, &ih, _1));

	ASSERT_TRUE(!rs.empty());
	ASSERT_EQ(rs.size(), 2u);
	ASSERT_EQ(ih.Items.size(), 2u);
	{
		int seq[] = {1, 42};
		ASSERT_TRUE(std::equal(ih.Items.begin(), ih.Items.end(), std::begin(seq), std::end(seq)));
		ASSERT_TRUE(std::equal(rs.begin(), rs.end(), std::begin(seq), std::end(seq)));
	}

	Set::iterator it = rs.find(42);
	ASSERT_TRUE(it != rs.end());
	rs.erase(it, Bind(&ItemsHolder::Remove, &ih, _1));

	ASSERT_TRUE(!rs.empty());
	ASSERT_EQ(rs.size(), 2u);
	ASSERT_EQ(ih.Items.size(), 2u);
	{
		int seq[] = {1, 42};
		ASSERT_TRUE(std::equal(ih.Items.begin(), ih.Items.end(), std::begin(seq), std::end(seq)));
		ASSERT_TRUE(std::equal(rs.begin(), rs.end(), std::begin(seq), std::end(seq)));
	}
}


TEST(RefCountingTest, Value)
{
	typedef RefCountingValue<int> Value;

	int v = 0;
	Value rv;

	ASSERT_EQ(rv.count(), 0u);

	ASSERT_EQ(rv.get(), null);
	ASSERT_EQ(v, 0);

	rv.set(Bind(make_assigner(v), 1));
	ASSERT_EQ(*rv.get(), 1);
	ASSERT_EQ(v, 1);

	rv.set(Bind(make_assigner(v), 2));
	ASSERT_EQ(*rv.get(), 1);
	ASSERT_EQ(v, 1);

	rv.release(Bind(make_assigner(v), 0));
	ASSERT_EQ(*rv.get(), 1);
	ASSERT_EQ(v, 1);

	rv.reset(Bind(make_assigner(v), 0));
	ASSERT_EQ(rv.get(), null);
	ASSERT_EQ(v, 0);
}


TEST(RefCountingTest, Counter)
{
	typedef RefCounter Counter;

	int v = 0;
	Counter rc;

	ASSERT_EQ(rc.count(), 0u);

	rc.set(Bind(make_assigner(v), 1));
	ASSERT_EQ(rc.count(), 1u);
	ASSERT_EQ(v, 1);

	rc.set(Bind(make_assigner(v), 2));
	ASSERT_EQ(rc.count(), 2u);
	ASSERT_EQ(v, 1);

	rc.release(Bind(make_assigner(v), 0));
	ASSERT_EQ(rc.count(), 1u);
	ASSERT_EQ(v, 1);

	rc.reset(Bind(make_assigner(v), 0));
	ASSERT_EQ(rc.count(), 0u);
	ASSERT_EQ(v, 0);
}
