#include <stingraykit/function/bind.h>
#include <stingraykit/thread/Thread.h>
#include <stingraykit/time/ElapsedTime.h>
#include <stingraykit/shared_ptr.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	struct SemHelper
	{
		STINGRAYKIT_NONCOPYABLE(SemHelper);

	public:
		Semaphore	Sem;

		Mutex		FlagMutex;
		int			Flag;

		SemHelper(int count) :
			Sem(count),
			Flag(0)
		{ }
	};
	STINGRAYKIT_DECLARE_PTR(SemHelper);


	void AquireSemaphoreFunc(const SemHelperPtr& helper)
	{
		helper->Sem.Wait();

		MutexLock l(helper->FlagMutex);
		++helper->Flag;
	}

}


TEST(SemsTest, DISABLED_Basic)
{
	const int SemValue = 3;
	SemHelperPtr semhelper = make_shared_ptr<SemHelper>(SemValue);

	const int ThreadCount = 10;
	std::vector<ThreadPtr> aquirers;
	for (int i = 0; i < ThreadCount; ++i)
		aquirers.push_back(make_shared_ptr<Thread>("SemTester", Bind(&AquireSemaphoreFunc, semhelper)));

	Thread::Sleep(1000);
	{
		MutexLock l(semhelper->FlagMutex);
		ASSERT_EQ(semhelper->Flag, SemValue);
	}

	const int SignalCount = 4;
	for (int i = 0; i < SignalCount; ++i)
		semhelper->Sem.Signal();


	Thread::Sleep(1000);
	{
		MutexLock l(semhelper->FlagMutex);
		ASSERT_EQ(semhelper->Flag, SemValue + SignalCount);
	}

	for (int i = 0; i < ThreadCount - SemValue - SignalCount; ++i)
		semhelper->Sem.Signal();

	Thread::Sleep(1000);
	{
		MutexLock l(semhelper->FlagMutex);
		ASSERT_EQ(semhelper->Flag, ThreadCount);
	}

	aquirers.clear();
}


TEST(SemaphoresTest, DISABLED_TimedWait)
{
	Semaphore sem;

	int intervals[] = { 100, 1000, 10000 };
	for (size_t i = 0; i < sizeof(intervals) / sizeof(intervals[0]); ++i)
	{
		ElapsedTime elapsed;
		bool success = sem.TimedWait(TimeDuration(intervals[i]));
		ASSERT_TRUE(!success);
		if (success)
			continue;

		ASSERT_LE(intervals[i], elapsed.ElapsedMilliseconds());
	}
}
