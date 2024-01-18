// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/function/bind.h>
#include <stingraykit/log/Logger.h>
#include <stingraykit/thread/atomic.h>
#include <stingraykit/thread/Thread.h>
#include <stingraykit/timer/Timer.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace stingray
{
	//STINGRAYKIT_TRACE_SHARED_PTRS(TimerDummy);
}

namespace
{

	struct TimerDummy
	{ };


	void SameTimeTimerFunc(u32 i, atomic<u32>& counter, bool& invalidOrder)
	{
		if (++counter != i)
			invalidOrder = true;
	}


	void WaitBlockedTimerFunc(Mutex& mutex, bool& blocked, ConditionVariable& cond)
	{
		MutexLock l(mutex);
		while (blocked)
			cond.Wait(mutex);
	}


	void NopFunc(const shared_ptr<TimerDummy>&) { }

}


TEST(TimerTest, SameTimeScheduleOrder)
{
	try
	{
		TimerPtr timer(new Timer("timerTest"));
		const u32 N = 10000;

		bool invalid_order = false;
		atomic<u32> counter(0);

		{
			ActionLogger al("Running " + ToString(N) + " functions");
			for (u32 i = 0; i < N; ++i)
				timer->AddTask(Bind(&SameTimeTimerFunc, i + 1, wrap_ref(counter), wrap_ref(invalid_order)));
			timer.reset();
		}

		Logger::Info() << "Executed " << counter << " functions";

		ASSERT_TRUE(!invalid_order);
		ASSERT_TRUE(counter == N);
	}
	catch (const std::exception& ex)
	{ Logger::Error() << STINGRAYKIT_WHERE << ": " << ex; }
	Logger::Info() << "TestSameTimeScheduleOrder completed";
}


TEST(TimerTest, SeparatedScheduleAndExecuting)
{
	try
	{
		TimerPtr timer(new Timer("timerTest"));
		const u32 N = 10000;

		bool invalid_order = false;
		atomic<u32> counter(0);

		{
			Mutex mutex;
			bool blocked = true;
			ConditionVariable cond;

			timer->AddTask(Bind(&WaitBlockedTimerFunc, wrap_ref(mutex), wrap_ref(blocked), wrap_ref(cond)));
			{
				ActionLogger al("Scheduling " + ToString(N) + " functions");
				for (u32 i = 0; i < N; ++i)
					timer->AddTask(Bind(&SameTimeTimerFunc, i + 1, wrap_ref(counter), wrap_ref(invalid_order)));
			}
			{
				MutexLock l(mutex);
				blocked = false;
				cond.Broadcast();
			}
			ActionLogger al("Executing " + ToString(N) + " functions");
			timer.reset();
		}

		Logger::Info() << "Executed " << counter << " functions";

		ASSERT_TRUE(!invalid_order);
		ASSERT_TRUE(counter == N);
	}
	catch (const std::exception& ex)
	{ Logger::Error() << STINGRAYKIT_WHERE << ": " << ex; }
	Logger::Info() << "TestSeparatedScheduleAndExecuting completed";
}


TEST(TimerTest, ScheduledFunctionDeath)
{
	Timer timer("timerTest");

	shared_ptr<TimerDummy> param = make_shared_ptr<TimerDummy>();
	for (int i = 0; i < 1000; ++i)
	{
		const int ConnectionsCount = 100;
		std::vector<Token> connections;
		for (int j = 0; j < ConnectionsCount; ++j)
			connections.push_back(timer.SetTimeout(TimeDuration(i), Bind(&NopFunc, param)));

		for (int j = 0; j < ConnectionsCount; ++j)
		{
			int index = (i + j) % ConnectionsCount;
			connections[index].Reset();
		}
		connections.clear();

		ASSERT_EQ(param.unique(), true);
	}
	Logger::Info() << "TestScheduledFunctionDeath completed";
}
