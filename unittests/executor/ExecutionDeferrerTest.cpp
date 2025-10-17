// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/executor/ExecutionDeferrer.h>
#include <stingraykit/executor/Timer.h>
#include <stingraykit/function/bind.h>
#include <stingraykit/function/functional.h>

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
		DeferrerHolder(ITimer& timer, TimeDuration timeout, ExecutionDeferrer::TaskType&& func)
			: _deferrer(timer, timeout)
		{ _deferrer.Defer(std::move(func)); }

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
	static void DoNothing() { }
};


TEST_F(ExecutionDeferrerTest, Cancel)
{
	const TimeDuration Timeout = TimeDuration::FromMilliseconds(500);
	const size_t ObjectsCount = 1000;

	Timer timer("deferrerTestTimer");
	Counter counter;

	for (size_t i = 0; i < ObjectsCount; ++i)
	{
		unique_ptr<DeferrerHolder> tmp(new DeferrerHolder(timer, Timeout, Bind(&Counter::Increment, wrap_ref(counter))));
		tmp.reset();
	}

	Thread::Sleep(Timeout * 2);
	ASSERT_EQ(counter.GetValue(), 0u);
}


TEST_F(ExecutionDeferrerTest, Defer)
{
	const TimeDuration EvenTimeout;
	const TimeDuration OddTimeout = TimeDuration::FromMilliseconds(200);
	const size_t TestCount = 10000;

	ExecutionDeferrerWithTimer deferrer("deferrerTestTimer");
	for (size_t i = 0; i < TestCount; ++i)
	{
		const TimeDuration timeout = i % 2 ? OddTimeout : EvenTimeout;
		deferrer.Defer(&ExecutionDeferrerTest::DoNothing, timeout);
	}
}

TEST_F(ExecutionDeferrerTest, TaskMoving)
{
	{
		Timer timer("deferrerTestTimer");
		ExecutionDeferrer deferrer(timer, TimeDuration::Hour());

		ITaskExecutor::TaskType task = NopFunctor();

		deferrer.Defer(task);
		ASSERT_NO_THROW(task());

		deferrer.Defer(std::move(task));
		ASSERT_ANY_THROW(task());
	}

	{
		ExecutionDeferrerWithTimer deferrer("deferrerTestTimer", TimeDuration::Hour());

		ITaskExecutor::TaskType task = NopFunctor();

		deferrer.Defer(task);
		ASSERT_NO_THROW(task());

		deferrer.Defer(std::move(task));
		ASSERT_ANY_THROW(task());
	}
}
