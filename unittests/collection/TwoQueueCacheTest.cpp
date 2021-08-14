#include <stingraykit/collection/TwoQueueCache.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	template < typename Key_, typename Value_ >
	optional<Value_> CacheGet(ICache<Key_, Value_>& cache, const Key_& key)
	{
		Value_ out;
		return cache.TryGet(key, out) ? make_optional_value(out) : null;
	}

}


TEST(TwoQueueCacheTest, DegenerativeSizes)
{
	typedef TwoQueueCache<s32, s32> Cache;

	{
		const size_t InQueueCapacity = 0;
		const size_t OutQueueCapacity = 0;
		const size_t HotCacheCapacity = 0;

		Cache cache(InQueueCapacity, OutQueueCapacity, HotCacheCapacity);
		ASSERT_EQ(cache.GetSize(), size_t(0));

		cache.Set(1, 1);
		cache.Set(2, 2);
		cache.Set(3, 3);

		ASSERT_EQ(cache.GetSize(), size_t(0));

		ASSERT_FALSE(CacheGet(cache, 1));
		ASSERT_FALSE(CacheGet(cache, 2));
		ASSERT_FALSE(CacheGet(cache, 3));
	}
	{
		const size_t InQueueCapacity = 1;
		const size_t OutQueueCapacity = 0;
		const size_t HotCacheCapacity = 0;

		Cache cache(InQueueCapacity, OutQueueCapacity, HotCacheCapacity);
		ASSERT_EQ(cache.GetSize(), size_t(0));

		cache.Set(1, 1);
		cache.Set(2, 2);
		cache.Set(3, 3);

		ASSERT_EQ(cache.GetSize(), size_t(1));

		ASSERT_FALSE(CacheGet(cache, 1));
		ASSERT_FALSE(CacheGet(cache, 2));
		ASSERT_EQ(*CacheGet(cache, 3), 3);

		ASSERT_EQ(cache.GetSize(), size_t(1));
	}
	{
		const size_t InQueueCapacity = 1;
		const size_t OutQueueCapacity = 0;
		const size_t HotCacheCapacity = 1;

		Cache cache(InQueueCapacity, OutQueueCapacity, HotCacheCapacity);
		ASSERT_EQ(cache.GetSize(), size_t(0));

		cache.Set(1, 1);
		cache.Set(2, 2);
		cache.Set(3, 3);

		ASSERT_EQ(cache.GetSize(), size_t(1));

		ASSERT_FALSE(CacheGet(cache, 1));
		ASSERT_FALSE(CacheGet(cache, 2));
		ASSERT_EQ(*CacheGet(cache, 3), 3);

		ASSERT_EQ(cache.GetSize(), size_t(1));
	}
	{
		const size_t InQueueCapacity = 0;
		const size_t OutQueueCapacity = 0;
		const size_t HotCacheCapacity = 1;

		Cache cache(InQueueCapacity, OutQueueCapacity, HotCacheCapacity);
		ASSERT_EQ(cache.GetSize(), size_t(0));

		cache.Set(1, 1);
		cache.Set(2, 2);
		cache.Set(3, 3);

		ASSERT_EQ(cache.GetSize(), size_t(0));

		ASSERT_FALSE(CacheGet(cache, 1));
		ASSERT_FALSE(CacheGet(cache, 2));
		ASSERT_FALSE(CacheGet(cache, 3));
	}
	{
		const size_t InQueueCapacity = 0;
		const size_t OutQueueCapacity = 1;
		const size_t HotCacheCapacity = 0;

		Cache cache(InQueueCapacity, OutQueueCapacity, HotCacheCapacity);
		ASSERT_EQ(cache.GetSize(), size_t(0));

		cache.Set(1, 1);
		cache.Set(2, 2);
		cache.Set(3, 3);

		ASSERT_EQ(cache.GetSize(), size_t(1));

		ASSERT_FALSE(CacheGet(cache, 1));
		ASSERT_FALSE(CacheGet(cache, 2));
		ASSERT_EQ(*CacheGet(cache, 3), 3);

		ASSERT_EQ(cache.GetSize(), size_t(0));

		ASSERT_FALSE(CacheGet(cache, 3));
	}
	{
		const size_t InQueueCapacity = 0;
		const size_t OutQueueCapacity = 1;
		const size_t HotCacheCapacity = 1;

		Cache cache(InQueueCapacity, OutQueueCapacity, HotCacheCapacity);
		ASSERT_EQ(cache.GetSize(), size_t(0));

		cache.Set(1, 1);
		cache.Set(2, 2);
		cache.Set(3, 3);

		ASSERT_EQ(cache.GetSize(), size_t(1));

		ASSERT_FALSE(CacheGet(cache, 1));
		ASSERT_FALSE(CacheGet(cache, 2));
		ASSERT_EQ(*CacheGet(cache, 3), 3);

		ASSERT_EQ(cache.GetSize(), size_t(1));

		ASSERT_EQ(*CacheGet(cache, 3), 3);
	}
	{
		const size_t InQueueCapacity = 1;
		const size_t OutQueueCapacity = 1;
		const size_t HotCacheCapacity = 0;

		Cache cache(InQueueCapacity, OutQueueCapacity, HotCacheCapacity);
		ASSERT_EQ(cache.GetSize(), size_t(0));

		cache.Set(1, 1);
		cache.Set(2, 2);
		cache.Set(3, 3);

		ASSERT_EQ(cache.GetSize(), size_t(2));

		ASSERT_FALSE(CacheGet(cache, 1));
		ASSERT_EQ(*CacheGet(cache, 2), 2);
		ASSERT_EQ(*CacheGet(cache, 3), 3);

		ASSERT_EQ(cache.GetSize(), size_t(1));

		ASSERT_FALSE(CacheGet(cache, 2));
		ASSERT_EQ(*CacheGet(cache, 3), 3);

		cache.Set(4, 4);

		ASSERT_EQ(cache.GetSize(), size_t(2));

		ASSERT_EQ(*CacheGet(cache, 3), 3);
		ASSERT_EQ(*CacheGet(cache, 4), 4);

		ASSERT_EQ(cache.GetSize(), size_t(1));

		ASSERT_FALSE(CacheGet(cache, 3));
		ASSERT_EQ(*CacheGet(cache, 4), 4);
	}
}


TEST(TwoQueueCacheTest, Eviction)
{
	typedef TwoQueueCache<s32, s32> Cache;

	const size_t InQueueCapacity = 5;
	const size_t OutQueueCapacity = 5;
	const size_t HotCacheCapacity = 5;

	Cache cache(InQueueCapacity, OutQueueCapacity, HotCacheCapacity);

	cache.Set(1, 1);
	cache.Set(2, 2);
	cache.Set(3, 3);
	cache.Set(4, 4);
	cache.Set(5, 5);
	cache.Set(6, 6);
	cache.Set(7, 7);
	cache.Set(8, 8);
	cache.Set(9, 9);
	cache.Set(10, 10);

	ASSERT_EQ(cache.GetSize(), size_t(10));

	cache.Set(11, 11);
	cache.Set(12, 12);
	cache.Set(13, 13);
	cache.Set(14, 14);
	cache.Set(15, 15);

	ASSERT_EQ(cache.GetSize(), size_t(10));

	ASSERT_FALSE(CacheGet(cache, 1));
	ASSERT_FALSE(CacheGet(cache, 2));
	ASSERT_FALSE(CacheGet(cache, 3));
	ASSERT_FALSE(CacheGet(cache, 4));
	ASSERT_FALSE(CacheGet(cache, 5));
	ASSERT_EQ(*CacheGet(cache, 6), 6);
	ASSERT_EQ(*CacheGet(cache, 7), 7);
	ASSERT_EQ(*CacheGet(cache, 8), 8);
	ASSERT_EQ(*CacheGet(cache, 9), 9);
	ASSERT_EQ(*CacheGet(cache, 10), 10);
	ASSERT_EQ(*CacheGet(cache, 11), 11);
	ASSERT_EQ(*CacheGet(cache, 12), 12);
	ASSERT_EQ(*CacheGet(cache, 13), 13);
	ASSERT_EQ(*CacheGet(cache, 14), 14);
	ASSERT_EQ(*CacheGet(cache, 15), 15);

	cache.Set(5, 5);
	cache.Set(4, 4);
	cache.Set(3, 3);
	cache.Set(2, 2);
	cache.Set(1, 1);

	ASSERT_EQ(cache.GetSize(), size_t(15));

	ASSERT_EQ(*CacheGet(cache, 10), 10);
	ASSERT_EQ(*CacheGet(cache, 9), 9);
	ASSERT_EQ(*CacheGet(cache, 8), 8);
	ASSERT_EQ(*CacheGet(cache, 7), 7);
	ASSERT_EQ(*CacheGet(cache, 6), 6);

	ASSERT_EQ(*CacheGet(cache, 15), 15);
	ASSERT_EQ(*CacheGet(cache, 13), 13);
	ASSERT_EQ(*CacheGet(cache, 11), 11);

	ASSERT_EQ(cache.GetSize(), size_t(12));

	ASSERT_FALSE(CacheGet(cache, 10));
	ASSERT_FALSE(CacheGet(cache, 9));
	ASSERT_FALSE(CacheGet(cache, 8));
	ASSERT_EQ(*CacheGet(cache, 7), 7);
	ASSERT_EQ(*CacheGet(cache, 6), 6);

	ASSERT_EQ(cache.GetSize(), size_t(12));

	ASSERT_TRUE(cache.TryRemove(5));
	ASSERT_TRUE(cache.TryRemove(1));
	ASSERT_FALSE(cache.TryRemove(5));
	ASSERT_FALSE(cache.TryRemove(1));

	ASSERT_EQ(cache.GetSize(), size_t(10));

	cache.Set(21, 21);
	cache.Set(22, 22);
	cache.Set(23, 23);
	cache.Set(24, 24);
	cache.Set(25, 25);

	ASSERT_EQ(cache.GetSize(), size_t(15));

	ASSERT_TRUE(cache.TryRemove(7));
	ASSERT_TRUE(cache.TryRemove(13));

	ASSERT_EQ(cache.GetSize(), size_t(13));

	ASSERT_EQ(*CacheGet(cache, 21), 21);
	ASSERT_EQ(*CacheGet(cache, 22), 22);
	ASSERT_EQ(*CacheGet(cache, 23), 23);
	ASSERT_EQ(*CacheGet(cache, 24), 24);
	ASSERT_EQ(*CacheGet(cache, 25), 25);

	ASSERT_EQ(cache.GetSize(), size_t(13));

	ASSERT_TRUE(cache.TryRemove(25));
	ASSERT_TRUE(cache.TryRemove(24));
	ASSERT_TRUE(cache.TryRemove(23));
	ASSERT_TRUE(cache.TryRemove(22));
	ASSERT_TRUE(cache.TryRemove(21));
	ASSERT_TRUE(cache.TryRemove(2));
	ASSERT_TRUE(cache.TryRemove(3));
	ASSERT_TRUE(cache.TryRemove(4));
	ASSERT_TRUE(cache.TryRemove(14));
	ASSERT_TRUE(cache.TryRemove(12));
	ASSERT_TRUE(cache.TryRemove(6));
	ASSERT_TRUE(cache.TryRemove(15));
	ASSERT_TRUE(cache.TryRemove(11));

	ASSERT_EQ(cache.GetSize(), size_t(0));
}


namespace
{

	struct StringSizeMapper
	{
		size_t operator()(const std::string& str) const
		{ return str.size(); }
	};

}


TEST(TwoQueueCacheTest, ZeroSizeMapping)
{
	typedef TwoQueueCache<s32, std::string, StringSizeMapper> Cache;

	const size_t Capacity = 0;

	Cache cache(Capacity, Capacity, Capacity);

	ASSERT_EQ(cache.GetSize(), size_t(0));
	cache.Set(1, "Ardent");
	ASSERT_EQ(cache.GetSize(), size_t(0));
	cache.Set(2, "");
	ASSERT_EQ(cache.GetSize(), size_t(0));
	cache.Set(3, "Swallows");
}


TEST(TwoQueueCacheTest, SizeMapping)
{
	typedef TwoQueueCache<s32, std::string, StringSizeMapper> Cache;

	const size_t InQueueCapacity = 10;
	const size_t OutQueueCapacity = 20;
	const size_t HotCacheCapacity = 10;

	Cache cache(InQueueCapacity, OutQueueCapacity, HotCacheCapacity);

	ASSERT_EQ(cache.GetSize(), size_t(0));
	cache.Set(1, "I");
	ASSERT_EQ(cache.GetSize(), size_t(1));
	cache.Set(2, "found you");
	ASSERT_EQ(cache.GetSize(), size_t(10));
	cache.Set(3, "");
	ASSERT_EQ(cache.GetSize(), size_t(10));
	cache.Set(4, "");
	ASSERT_EQ(cache.GetSize(), size_t(10));
	cache.Set(5, "with drift wood");
	ASSERT_EQ(cache.GetSize(), size_t(15));
	cache.Set(6, "tamped ");
	ASSERT_EQ(cache.GetSize(), size_t(22));
	cache.Set(7, "inside you");
	ASSERT_EQ(cache.GetSize(), size_t(17));
	cache.Set(8, "You");
	ASSERT_EQ(cache.GetSize(), size_t(20));
	cache.Set(9, "show great");
	ASSERT_EQ(cache.GetSize(), size_t(30));
	cache.Set(10, "");
	ASSERT_EQ(cache.GetSize(), size_t(30));
	cache.Set(11, " ");
	ASSERT_EQ(cache.GetSize(), size_t(14));
	ASSERT_EQ(*CacheGet(cache, 11), " ");
	ASSERT_EQ(*CacheGet(cache, 8), "You");
	cache.Set(12, "-------------------------------------------------");
	ASSERT_EQ(cache.GetSize(), size_t(3));
	ASSERT_EQ(*CacheGet(cache, 8), "You");
	cache.Set(1, "--------------------");
	ASSERT_EQ(cache.GetSize(), size_t(23));
	ASSERT_EQ(*CacheGet(cache, 1), "--------------------");
	ASSERT_EQ(cache.GetSize(), size_t(0));
}
