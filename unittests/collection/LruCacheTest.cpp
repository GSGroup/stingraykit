#include <stingraykit/collection/QueueCache.h>

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


TEST(LruCacheTest, EmptyCache)
{
	typedef LruCache<s32, s32>::ValueT Cache;

	const size_t Capacity = 0;

	Cache cache(Capacity);
	ASSERT_EQ(cache.GetSize(), size_t(0));

	cache.Set(1, 1);
	cache.Set(2, 2);
	cache.Set(3, 3);
	cache.Set(4, 4);
	cache.Set(5, 5);

	ASSERT_EQ(cache.GetSize(), size_t(0));

	ASSERT_FALSE(CacheGet(cache, 1));
	ASSERT_FALSE(CacheGet(cache, 2));
	ASSERT_FALSE(CacheGet(cache, 3));
	ASSERT_FALSE(CacheGet(cache, 4));
	ASSERT_FALSE(CacheGet(cache, 5));
}


TEST(LruCacheTest, Filling)
{
	typedef LruCache<s32, s32>::ValueT Cache;

	const size_t Capacity = 10;

	Cache cache(Capacity);
	ASSERT_EQ(cache.GetSize(), size_t(0));

	cache.Set(1, 1);
	cache.Set(2, 2);
	cache.Set(3, 3);
	cache.Set(4, 4);
	cache.Set(5, 5);

	ASSERT_EQ(cache.GetSize(), size_t(5));

	cache.Set(4, 5);
	cache.Set(5, 6);

	ASSERT_EQ(cache.GetSize(), size_t(5));

	cache.Set(6, 6);
	cache.Set(7, 7);
	cache.Set(8, 8);
	cache.Set(9, 9);
	cache.Set(10, 10);

	ASSERT_EQ(cache.GetSize(), size_t(10));

	ASSERT_EQ(*CacheGet(cache, 1), 1);
	ASSERT_EQ(*CacheGet(cache, 2), 2);
	ASSERT_EQ(*CacheGet(cache, 3), 3);
	ASSERT_EQ(*CacheGet(cache, 4), 5);
	ASSERT_EQ(*CacheGet(cache, 5), 6);
	ASSERT_EQ(*CacheGet(cache, 6), 6);
	ASSERT_EQ(*CacheGet(cache, 7), 7);
	ASSERT_EQ(*CacheGet(cache, 8), 8);
	ASSERT_EQ(*CacheGet(cache, 9), 9);
	ASSERT_EQ(*CacheGet(cache, 10), 10);
}


TEST(LruCacheTest, Eviction)
{
	typedef LruCache<s32, s32>::ValueT Cache;

	const size_t Capacity = 10;

	Cache cache(Capacity);

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

	ASSERT_EQ(*CacheGet(cache, 15), 15);
	ASSERT_EQ(*CacheGet(cache, 14), 14);
	ASSERT_EQ(*CacheGet(cache, 13), 13);
	ASSERT_EQ(*CacheGet(cache, 12), 12);
	ASSERT_EQ(*CacheGet(cache, 11), 11);
	ASSERT_EQ(*CacheGet(cache, 10), 10);
	ASSERT_EQ(*CacheGet(cache, 9), 9);
	ASSERT_EQ(*CacheGet(cache, 8), 8);
	ASSERT_EQ(*CacheGet(cache, 7), 7);
	ASSERT_EQ(*CacheGet(cache, 6), 6);
	ASSERT_FALSE(CacheGet(cache, 5));
	ASSERT_FALSE(CacheGet(cache, 4));
	ASSERT_FALSE(CacheGet(cache, 3));
	ASSERT_FALSE(CacheGet(cache, 2));
	ASSERT_FALSE(CacheGet(cache, 1));

	cache.Set(5, 5);
	cache.Set(4, 4);
	cache.Set(3, 3);
	cache.Set(2, 2);
	cache.Set(1, 1);

	ASSERT_EQ(cache.GetSize(), size_t(10));

	ASSERT_FALSE(CacheGet(cache, 15));
	ASSERT_FALSE(CacheGet(cache, 14));
	ASSERT_FALSE(CacheGet(cache, 13));
	ASSERT_FALSE(CacheGet(cache, 12));
	ASSERT_FALSE(CacheGet(cache, 11));
	ASSERT_EQ(*CacheGet(cache, 5), 5);
	ASSERT_EQ(*CacheGet(cache, 4), 4);
	ASSERT_EQ(*CacheGet(cache, 3), 3);
	ASSERT_EQ(*CacheGet(cache, 2), 2);
	ASSERT_EQ(*CacheGet(cache, 1), 1);
	ASSERT_EQ(*CacheGet(cache, 10), 10);
	ASSERT_EQ(*CacheGet(cache, 9), 9);
	ASSERT_EQ(*CacheGet(cache, 8), 8);
	ASSERT_EQ(*CacheGet(cache, 7), 7);
	ASSERT_EQ(*CacheGet(cache, 6), 6);

	ASSERT_TRUE(cache.TryRemove(2));
	ASSERT_TRUE(cache.TryRemove(4));
	ASSERT_TRUE(cache.TryRemove(6));
	ASSERT_FALSE(cache.TryRemove(4));
	ASSERT_FALSE(cache.TryRemove(6));
	ASSERT_FALSE(cache.TryRemove(21));

	ASSERT_EQ(cache.GetSize(), size_t(7));

	cache.Set(2, 2);
	cache.Set(4, 4);
	cache.Set(6, 6);
	cache.Set(8, 8);
	cache.Set(10, 10);
	cache.Set(12, 12);
	cache.Set(17, 17);
	cache.Set(16, 16);
	cache.Set(15, 15);

	ASSERT_EQ(cache.GetSize(), size_t(10));

	ASSERT_FALSE(CacheGet(cache, 1));
	ASSERT_FALSE(CacheGet(cache, 3));
	ASSERT_FALSE(CacheGet(cache, 5));
	ASSERT_FALSE(CacheGet(cache, 9));
	ASSERT_FALSE(CacheGet(cache, 11));
	ASSERT_FALSE(CacheGet(cache, 13));
	ASSERT_FALSE(CacheGet(cache, 14));
	ASSERT_EQ(*CacheGet(cache, 17), 17);
	ASSERT_EQ(*CacheGet(cache, 15), 15);
	ASSERT_EQ(*CacheGet(cache, 2), 2);
	ASSERT_EQ(*CacheGet(cache, 6), 6);
	ASSERT_EQ(*CacheGet(cache, 10), 10);
	ASSERT_EQ(*CacheGet(cache, 12), 12);
	ASSERT_EQ(*CacheGet(cache, 8), 8);
	ASSERT_EQ(*CacheGet(cache, 4), 4);
	ASSERT_EQ(*CacheGet(cache, 7), 7);
	ASSERT_EQ(*CacheGet(cache, 16), 16);

	ASSERT_FALSE(cache.TryRemove(1));
	ASSERT_TRUE(cache.TryRemove(2));
	ASSERT_FALSE(cache.TryRemove(3));
	ASSERT_TRUE(cache.TryRemove(4));
	ASSERT_FALSE(cache.TryRemove(5));
	ASSERT_TRUE(cache.TryRemove(6));
	ASSERT_TRUE(cache.TryRemove(7));

	ASSERT_EQ(cache.GetSize(), size_t(6));

	cache.Set(20, 20);
	cache.Set(21, 21);
	cache.Set(22, 22);
	cache.Set(23, 23);
	cache.Set(24, 24);
	cache.Set(25, 25);
	cache.Set(26, 26);
	cache.Set(27, 27);

	ASSERT_EQ(cache.GetSize(), size_t(10));

	ASSERT_FALSE(CacheGet(cache, 17));
	ASSERT_FALSE(CacheGet(cache, 15));
	ASSERT_FALSE(CacheGet(cache, 10));
	ASSERT_FALSE(CacheGet(cache, 12));
	ASSERT_EQ(*CacheGet(cache, 8), 8);
	ASSERT_EQ(*CacheGet(cache, 16), 16);
	ASSERT_EQ(*CacheGet(cache, 20), 20);
	ASSERT_EQ(*CacheGet(cache, 21), 21);
	ASSERT_EQ(*CacheGet(cache, 22), 22);
	ASSERT_EQ(*CacheGet(cache, 23), 23);
	ASSERT_EQ(*CacheGet(cache, 24), 24);
	ASSERT_EQ(*CacheGet(cache, 25), 25);
	ASSERT_EQ(*CacheGet(cache, 26), 26);
	ASSERT_EQ(*CacheGet(cache, 27), 27);

	ASSERT_TRUE(cache.TryRemove(27));
	ASSERT_TRUE(cache.TryRemove(25));
	ASSERT_TRUE(cache.TryRemove(23));
	ASSERT_TRUE(cache.TryRemove(21));
	ASSERT_TRUE(cache.TryRemove(16));
	ASSERT_TRUE(cache.TryRemove(8));
	ASSERT_TRUE(cache.TryRemove(20));
	ASSERT_TRUE(cache.TryRemove(22));
	ASSERT_TRUE(cache.TryRemove(24));
	ASSERT_TRUE(cache.TryRemove(26));

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


TEST(LruCacheTest, ZeroSizeMapping)
{
	typedef LruCache<s32, std::string, StringSizeMapper>::ValueT Cache;

	const size_t Capacity = 0;

	Cache cache(Capacity);

	ASSERT_EQ(cache.GetSize(), size_t(0));
	cache.Set(1, "Ardent");
	ASSERT_EQ(cache.GetSize(), size_t(0));
	cache.Set(2, "");
	ASSERT_EQ(cache.GetSize(), size_t(0));
	cache.Set(3, "Swallows");
}


TEST(LruCacheTest, SizeMapping)
{
	typedef LruCache<s32, std::string, StringSizeMapper>::ValueT Cache;

	const size_t Capacity = 30;

	Cache cache(Capacity);

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
	ASSERT_EQ(cache.GetSize(), size_t(25));
	cache.Set(6, "tamped ");
	ASSERT_EQ(cache.GetSize(), size_t(22));
	cache.Set(7, "inside you");
	ASSERT_EQ(cache.GetSize(), size_t(17));
	cache.Set(8, "You show grea");
	ASSERT_EQ(cache.GetSize(), size_t(30));
	cache.Set(9, "t spirit");
	ASSERT_EQ(cache.GetSize(), size_t(21));
	cache.Set(9, ".");
	ASSERT_EQ(cache.GetSize(), size_t(14));
	cache.Set(8, "");
	ASSERT_EQ(cache.GetSize(), size_t(1));
	cache.Set(9, "");
	ASSERT_EQ(cache.GetSize(), size_t(0));
	cache.TryRemove(8);
	ASSERT_EQ(cache.GetSize(), size_t(0));
	cache.TryRemove(9);
	ASSERT_EQ(cache.GetSize(), size_t(0));
}