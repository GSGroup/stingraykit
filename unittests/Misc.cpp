#include <unittests/mocks/Destructible.h>
#include <unittests/mocks/Invokable.h>

#include <stingraykit/function/bind.h>
#include <stingraykit/log/Logger.h>
#include <stingraykit/thread/Thread.h>
#include <stingraykit/signal/signals.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace stingray;
using namespace stingray::mock;


TEST(Misc, BindTest)
{
	MockDestructiblePtr destructible = make_shared_ptr<MockDestructible>();
	EXPECT_CALL(*destructible, Die());

	Bind(&Destructible::Invoke, destructible)();

	destructible = make_shared_ptr<MockDestructible>();
	EXPECT_CALL(*destructible, Die());

	function<void()>(Bind(&Destructible::Invoke, destructible))();
}


TEST(Misc, ThreadInterruptTest)
{
	struct ThreadFunc
	{
		static void Invoke(const DestructiblePtr& destructible)
		{
			DestructiblePtr local = destructible;
			for (;;)
				Thread::Sleep(10000);
		}
	};

	MockDestructiblePtr destructible = make_shared_ptr<MockDestructible>();
	EXPECT_CALL(*destructible, Die());

	ThreadPtr t = make_shared_ptr<Thread>("threadInterruptTester", Bind(&ThreadFunc::Invoke, destructible));
	Thread::Sleep(100);
	t->Interrupt();
	t.reset();
}


TEST(Misc, SignalsTest)
{
	stingray::signal<void()> s;

	MockInvokable invokable;
	EXPECT_CALL(invokable, Invoke()).Times(1);

	Token c = s.connect(Bind(&Invokable::Invoke, wrap_ref(invokable)));
	s();
	c.Reset();
	s();
}
