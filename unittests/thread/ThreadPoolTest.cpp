#include <stingraykit/function/bind.h>
#include <stingraykit/thread/ThreadPool.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	void SleepAndPush(u32 sleepMs, std::vector<u8>& vec, u8 num, const ICancellationToken& token)
	{
		token.Sleep(TimeDuration(sleepMs));
		if (token)
			vec.push_back(num);
	}

}


TEST(ThreadPoolTest, DISABLED_Order)
{
	ThreadPoolPtr workers(new ThreadPool("testPool", IntTraits<u32>::Max));
	u8 check[] = { 1, 2, 3 };
	std::vector<u8> results;
	workers->Queue(Bind(&SleepAndPush, 1000, wrap_ref(results), 1, _1));
	workers->Queue(Bind(&SleepAndPush, 3500, wrap_ref(results), 3, _1));
	Thread::Sleep(1500);
	workers->Queue(Bind(&SleepAndPush, 1000, wrap_ref(results), 2, _1));
	Thread::Sleep(2500);
	ASSERT_TRUE(results.size() == ArraySize(check) && std::equal(check, check + sizeof(check), results.begin()));
}


TEST(ThreadPoolTest, DISABLED_Destruction)
{
	u8 check[] = { 1, 2 };
	std::vector<u8> results;
	{
		ThreadPoolPtr workers(new ThreadPool("testPool", IntTraits<u32>::Max));
		workers->Queue(Bind(&SleepAndPush, 1000, wrap_ref(results), 1, _1));
		workers->Queue(Bind(&SleepAndPush, 3500, wrap_ref(results), 3, _1));
		Thread::Sleep(1500);
		workers->Queue(Bind(&SleepAndPush, 1000, wrap_ref(results), 2, _1));
		Thread::Sleep(1500);
	}
	ASSERT_TRUE(results.size() == ArraySize(check) && std::equal(check, check + sizeof(check), results.begin()));
}
