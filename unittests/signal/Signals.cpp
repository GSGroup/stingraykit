#include <unittests/Dummy.h>

#include <stingraykit/signal/signals.h>
#include <stingraykit/thread/ThreadlessTaskExecutor.h>
#include <stingraykit/thread/ThreadTaskExecutor.h>
#include <stingraykit/thread/Thread.h>

#include <gtest/gtest.h>

using namespace stingray;

class SignalsTest : public testing::Test, public FireRange {
protected:
	ThreadlessTaskExecutorPtr Worker;

public:
	SignalsTest() : Worker(new ThreadlessTaskExecutor("signalsTest", ThreadlessTaskExecutor::DefaultProfileTimeout, &FireRange::HandleException)) { }

	void Reset()
	{
		Worker->ExecuteTasks();
		Counter = 0;
	}

	void Slot(const Dummy&) { ASSERT_GE(Counter, 1); }

	static void Nop() { }

	static void StressThreadFunc(stingray::signal<void()>& signal)
	{
		for(size_t i = 0; i < 10000; ++i)
		{
			Token c = signal.connect(&SignalsTest::Nop);
			signal();
		}
	}

#if 0
	struct Hole {
		int value;
		Hole(int value = 0): value(value) {}
	};

	void ValueSlot(Hole &hole) {
		hole.value = 0xaa55aa55;
	}

	void _testInvariancy()
	{
		stingray::signal<void (Hole&)> signal;
		Hole hole(0x0badc0de);
		Token connection = signal.connect(Bind(&SignalsTest::ValueSlot, this, _1));
		signal(hole);
		ASSERT_EQ(hole.value, 0x0badc0de);
	}
#endif

};


TEST_F(SignalsTest, SignalByValue)
{
	stingray::signal<void (Dummy)> SignalValue;
	Token connection = SignalValue.connect(Worker, Bind(&SignalsTest::Slot, this, _1));

	Reset();
	ASSERT_EQ(Counter, 0);
	SignalValue(Dummy(this));
	//TS_TRACE("SIGNALLED\n");
	ASSERT_GE(Counter, 1);
	Worker->ExecuteTasks();
	ASSERT_EQ(Counter, 0);
}


TEST_F(SignalsTest, SignalByRef)
{
	stingray::signal<void (const Dummy&)> SignalRef;
	Token connection = SignalRef.connect(Worker, Bind(&SignalsTest::Slot, this, _1));

	Reset();
	ASSERT_EQ(Counter, 0);
	SignalRef(Dummy(this));
	ASSERT_GE(Counter, 1);
	Worker->ExecuteTasks();
	ASSERT_EQ(Counter, 0);
}


TEST_F(SignalsTest, ThreadedSignalByRef)
{
	ThreadTaskExecutorPtr worker(new ThreadTaskExecutor("signals-test-thread", ThreadTaskExecutor::DefaultProfileTimeout, &FireRange::HandleException));
	stingray::signal<void (const Dummy&)> SignalRef;
	Token connection = SignalRef.connect(worker, Bind(&SignalsTest::Slot, this, _1));

	ASSERT_EQ(Counter, 0);
	SignalRef(Dummy(this));
	Thread::Sleep(100); //fixme: FIX THIS!
	worker.reset();

	ASSERT_EQ(Counter, 0);
}


TEST_F(SignalsTest, ConnectionDisconnection)
{
	stingray::signal<void ()> signal;
	ThreadPtr t1 = make_shared_ptr<Thread>("signalStress1", Bind(&SignalsTest::StressThreadFunc, wrap_ref(signal)));
	ThreadPtr t2 = make_shared_ptr<Thread>("signalStress2", Bind(&SignalsTest::StressThreadFunc, wrap_ref(signal)));
	ThreadPtr t3 = make_shared_ptr<Thread>("signalStress3", Bind(&SignalsTest::StressThreadFunc, wrap_ref(signal)));
	ThreadPtr t4 = make_shared_ptr<Thread>("signalStress4", Bind(&SignalsTest::StressThreadFunc, wrap_ref(signal)));
	t1.reset();
	t2.reset();
	t3.reset();
	t4.reset();
}
