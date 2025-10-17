// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/executor/ThreadPool.h>
#include <stingraykit/function/bind.h>
#include <stingraykit/function/functional.h>
#include <stingraykit/thread/DummyCancellationToken.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	void SleepAndPush(u32 sleepMs, std::vector<u8>& vec, u8 num, const ICancellationToken& token)
	{
		token.Sleep(TimeDuration::FromMilliseconds(sleepMs));
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

TEST(ThreadPoolTest, TaskMoving)
{
	const ThreadPoolPtr pool = make_shared_ptr<ThreadPool>("testPool", 10);

	{
		ThreadPool::Task task = NopFunctor();

		pool->Queue(task);
		ASSERT_NO_THROW(task(DummyCancellationToken()));

		pool->Queue(std::move(task));
		ASSERT_ANY_THROW(task(DummyCancellationToken()));
	}

	{
		ThreadPool::Task task = NopFunctor();

		ASSERT_TRUE(pool->TryQueue(task));
		ASSERT_NO_THROW(task(DummyCancellationToken()));

		ASSERT_TRUE(pool->TryQueue(std::move(task)));
		ASSERT_ANY_THROW(task(DummyCancellationToken()));
	}

	{
		ThreadPool::Task task = NopFunctor();

		pool->WaitQueue(task, DummyCancellationToken());
		ASSERT_NO_THROW(task(DummyCancellationToken()));

		pool->WaitQueue(std::move(task), DummyCancellationToken());
		ASSERT_ANY_THROW(task(DummyCancellationToken()));
	}
}
