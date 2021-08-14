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
