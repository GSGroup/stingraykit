// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/function/bind.h>
#include <stingraykit/log/Logger.h>
#include <stingraykit/thread/atomic.h>
#include <stingraykit/thread/ThreadTaskExecutor.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	void IncrementCounterFunc(atomic<u32>& counter)
	{ ++counter; }


	void WaitBlockedExecutorFunc(Mutex& mutex, bool& blocked, ConditionVariable& cond)
	{
		MutexLock l(mutex);
		while (blocked)
			cond.Wait(mutex);
	}

}


TEST(TaskExecutorTest, DISABLED_SeparatedScheduleAndExecuting)
{
	try
	{
		ITaskExecutorPtr executor = make_shared_ptr<ThreadTaskExecutor>("executorTest");
		const u32 N = 10000;

		atomic<u32> counter(0);

		{
			Mutex mutex;
			bool blocked = true;
			ConditionVariable cond;

			executor->AddTask(Bind(&WaitBlockedExecutorFunc, wrap_ref(mutex), wrap_ref(blocked), wrap_ref(cond)));
			{
				ActionLogger al("Scheduling " + ToString(N) + " functions");
				for (u32 i = 0; i < N; ++i)
					executor->AddTask(Bind(&IncrementCounterFunc, wrap_ref(counter)));
			}
			{
				MutexLock l(mutex);
				blocked = false;
				cond.Broadcast();
			}
			ActionLogger al("Executing " + ToString(N) + " functions");
			executor.reset();
		}

		Logger::Info() << "Executed " << counter << " functions";

		ASSERT_TRUE(counter == N);
	}
	catch (const std::exception& ex)
	{ Logger::Error() << STINGRAYKIT_WHERE << ": " << ex; }
	Logger::Info() << "TestSeparatedScheduleAndExecuting completed";
}
