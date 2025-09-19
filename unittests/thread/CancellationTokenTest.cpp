// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/function/bind.h>
#include <stingraykit/thread/CancellationRegistrator.h>
#include <stingraykit/thread/CancellationToken.h>
#include <stingraykit/thread/Thread.h>

#include <gtest/gtest.h>

using namespace stingray;

class CancellationTokenTest : public testing::Test
{
protected:
	Mutex				_mutex;
	ConditionVariable	_cond;
	bool				_cancelDone;

public:
	void ThreadFunc(const ICancellationToken& token)
	{
		Mutex _mutex;
		ConditionVariable wt;

		MutexLock l(_mutex);
		wt.Wait(_mutex, token);
	}

	struct CancellationHandler final : public virtual ICancellationHandler
	{
	private:
		bool&	_flag;

	public:
		CancellationHandler(bool& flag) : _flag(flag)
		{ }

		~CancellationHandler() override
		{ }

		void Cancel() override
		{ _flag = true; }
	};

	void ThreadFunc2(TimeDuration sleepDuration, const ICancellationToken& token)
	{
		Thread::Sleep(sleepDuration);

		bool cancelled = false;
		bool handlerCancelled = false;
		{
			CancellationHandler handler(wrap_ref(handlerCancelled));
			CancellationRegistrator registrator(token, handler);
			cancelled = registrator.IsCancelled();

			MutexLock l(_mutex);
			while (!_cancelDone)
				_cond.Wait(_mutex);
		}
		ASSERT_TRUE(cancelled || handlerCancelled);
	}
};


TEST_F(CancellationTokenTest, DISABLED_WaitToken)
{
	CancellationToken token;
	ThreadPtr thread = make_shared_ptr<Thread>("cancellationTokenTest", Bind(&CancellationTokenTest::ThreadFunc, this, wrap_const_ref(token)));
	token.Cancel();
	thread.reset();
}


TEST_F(CancellationTokenTest, DISABLED_Handler)
{
	const int Count = 200;
	for (int i = 0; i < Count; i += 5)
	{
		_cancelDone = false;

		TimeDuration sleepDuration = TimeDuration::FromMilliseconds(i);
		TimeDuration otherThreadSleepDuration = TimeDuration::FromMilliseconds(Count - i);
		CancellationToken token;
		ThreadPtr thread = make_shared_ptr<Thread>("cancellationTokenTest", Bind(&CancellationTokenTest::ThreadFunc2, this, otherThreadSleepDuration, wrap_const_ref(token)));

		Thread::Sleep(sleepDuration);

		token.Cancel();
		{
			MutexLock l(_mutex);
			_cancelDone = true;
			_cond.Broadcast();
		}

		thread.reset();
	}
}


class ConditionVariableCancellationTest : public testing::Test
{
protected:
	class CustomCancellationToken final : public virtual ICancellationToken
	{
	private:
		CancellationToken						_token;
		mutable atomic<u32>						_sleepDuration;
		mutable ProxyCancellationRegistrator	_registrator;

	public:
		CustomCancellationToken()
			: _sleepDuration(0), _registrator(_token)
		{ }

		bool Sleep(optional<TimeDuration> duration) const override
		{ return _token.Sleep(duration); }

		bool IsCancelled() const override
		{ return _token.IsCancelled(); }

		bool IsTimedOut() const override
		{ return _token.IsTimedOut(); }

		optional<TimeDuration> GetTimeout() const override
		{ return _token.GetTimeout(); }

		void Cancel()
		{ _token.Cancel(); }

	private:
		bool TryRegisterCancellationHandler(ICancellationHandler& handler) const override
		{
			_sleepDuration = 200;
			return _registrator.TryRegisterCancellationHandler(handler);
		}

		bool TryUnregisterCancellationHandler() const override
		{
			Thread::Sleep(_sleepDuration);
			_sleepDuration = 0;

			return _registrator.TryUnregisterCancellationHandler();
		}

		bool UnregisterCancellationHandler() const override
		{
			_sleepDuration = 0;
			return _registrator.UnregisterCancellationHandler();
		}
	};

protected:
	Mutex				_mutex;
	ConditionVariable	_cond;
	ThreadPtr			_worker;

public:
	void ThreadFunc(const ICancellationToken& token)
	{
		MutexLock l(_mutex);
		while (token)
			_cond.Wait(_mutex, token);
	}
};


TEST_F(ConditionVariableCancellationTest, DISABLED_ConditionCancel)
{
	const int Count = 50;
	for (int i = 0; i < Count; ++i)
	{
		CustomCancellationToken token;
		ThreadPtr thread = make_shared_ptr<Thread>("condVarCancelTemp", Bind(&ConditionVariableCancellationTest::ThreadFunc, this, wrap_const_ref(token)));

		Thread::Sleep(100);
		{
			MutexLock l(_mutex);
			_cond.Broadcast();
		}
		Thread::Sleep(100);
		token.Cancel();
		thread.reset();
	}
}
