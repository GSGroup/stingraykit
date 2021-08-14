#include <stingraykit/function/bind.h>
#include <stingraykit/thread/SyncValue.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	void incrementer(SyncValue<int>& value)
	{
		for (int i = 0; i < 500; ++i)
			++*value;
	}

	void decrementer(SyncValue<int>& value)
	{
		for (int i = 0; i < 500; ++i)
			--*value;
	}

	struct Foo
	{
		void Set() { }
		void Get() const { }
	};

}

TEST(SyncValueTest, DISABLED_ThreadSafety)
{
	SyncValue<int> testee(0);
	{
		Thread t1("incrementer", Bind(&incrementer, wrap_ref(testee)));
		Thread t2("decrementer", Bind(&decrementer, wrap_ref(testee)));
	}
	ASSERT_TRUE(*testee == 0);
}

TEST(SyncValueTest, DISABLED_LockCvQualifiers)
{
	SyncValue<Foo> testee1;

	SyncValue<Foo>::Lock l1(testee1);
	l1->Get();
	l1->Set();

	const SyncValue<Foo>::Lock l2(testee1);
	l2->Get();
	// l2->Set(); compilation fail

	SyncValue<Foo>::ConstLock l3(testee1);
	l3->Get();
	// l3->Set(); compilation fail

	const SyncValue<Foo> testee2;

	// SyncValue<Foo>::Lock l4(testee2); compilation fail
	// const SyncValue<Foo>::Lock l5(testee2); compilation fail
	SyncValue<Foo>::ConstLock l6(testee2);
	l6->Get();
	// l6->Set(); compilation fail
}

TEST(SyncValueTest, DISABLED_OperatorArrow)
{
	SyncValue<Foo> testee;
	testee->Set();
}

TEST(SyncValueTest, DISABLED_AssignOperator)
{
	SyncValue<int> testee;
	testee = 1;
	ASSERT_TRUE(*testee == 1);
	*testee = 2;
	ASSERT_TRUE(*testee == 2);
}
