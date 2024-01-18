// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/function/bind.h>
#include <stingraykit/timer/Timer.h>
#include <stingraykit/unique_ptr.h>

#include <gtest/gtest.h>

using namespace stingray;

class ExecutionDeferrerTest : public testing::Test
{
protected:
	class DeferrerHolder
	{
	private:
		ExecutionDeferrer	_deferrer;

	public:
		DeferrerHolder(Timer& timer, size_t timeout, const function<void ()>& func)
			: _deferrer(timer, TimeDuration(timeout))
		{ _deferrer.Defer(func); }

		~DeferrerHolder()
		{ _deferrer.Cancel(); }
	};

	struct Counter
	{
	private:
		size_t	_value;

	public:
		Counter() : _value(0) { }

		size_t GetValue() const { return _value; }

		void Increment() { ++_value; }
	};

	struct ExecutionDeferrerTestDummy
	{ };
	STINGRAYKIT_DECLARE_PTR(ExecutionDeferrerTestDummy);

protected:
	bool _finished;

protected:
	ExecutionDeferrerTest() : _finished(false)
	{ }

	static void DoNothing() { }

	void DoCancel(const ExecutionDeferrerWithTimerPtr& deferrer)
	{ deferrer->Cancel(); }

public:
	void DoTest(size_t testCount, size_t timeout, size_t sleepTimeout)
	{
		ExecutionDeferrerWithTimerPtr deferrer(new ExecutionDeferrerWithTimer("deferrerTestTimer"));
		ExecutionDeferrerWithTimerPtr cancelDeferrer(new ExecutionDeferrerWithTimer("deferrerCancelTestTimer"));

		for (size_t i = 0; i < testCount; ++i)
		{
			deferrer->Defer(Bind(&ExecutionDeferrerTest::DoCancel, this, deferrer), TimeDuration(timeout));
			cancelDeferrer->Defer(Bind(&ExecutionDeferrerTest::DoCancel, this, deferrer), TimeDuration(timeout));
			Thread::Sleep(sleepTimeout);
		}

		deferrer.reset();
		cancelDeferrer.reset();
		_finished = true;
	}
};


TEST_F(ExecutionDeferrerTest, Cancel)
{
	const size_t Timeout		= 500;
	const size_t ObjectsCount	= 1000;

	Timer timer("deferrerTestTimer");
	Counter counter;

	for (size_t i = 0; i < ObjectsCount; ++i)
	{
		unique_ptr<DeferrerHolder> tmp(new DeferrerHolder(timer, Timeout, Bind(&Counter::Increment, wrap_ref(counter))));
		tmp.reset();
	}

	Thread::Sleep(2 * Timeout);
	ASSERT_EQ(counter.GetValue(), 0u);
}


TEST_F(ExecutionDeferrerTest, Defer)
{
	const size_t EvenTimeout 	= 0;
	const size_t OddTimeout		= 200;
	const size_t TestCount		= 10000;

	ExecutionDeferrerWithTimer deferrer("deferrerTestTimer");
	for (size_t i = 0; i < TestCount; ++i)
	{
		const size_t timeout = i % 2? OddTimeout : EvenTimeout;
		deferrer.Defer(&ExecutionDeferrerTest::DoNothing, TimeDuration(timeout));
	}
}


TEST_F(ExecutionDeferrerTest, CancelDeadlock)
{
	const size_t TestCount		= 100;
	const size_t Timeout		= 3;
	const size_t SleepTimeout	= 50;

	static ExecutionDeferrerWithTimer deferrer("deadlockTestDeferrerTimer");

	deferrer.Defer(Bind(&ExecutionDeferrerTest::DoTest, this, TestCount, Timeout, SleepTimeout), TimeDuration(0));

	size_t elapsed = 0;
	while(elapsed < (TestCount * SleepTimeout * 2) && !_finished)
	{
		elapsed += SleepTimeout;
		Thread::Sleep(SleepTimeout);
	}

	if (!_finished)
		STINGRAYKIT_THROW("Cannot finish ExecutionDeferrerTest, possible there is a deadlock");
}
