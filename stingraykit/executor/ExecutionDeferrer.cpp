// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/executor/ExecutionDeferrer.h>

#include <stingraykit/executor/Timer.h>
#include <stingraykit/function/bind.h>

namespace stingray
{

	ExecutionDeferrer::ExecutionDeferrer(ITimer& timer, optional<TimeDuration> timeout)
		: _timer(timer), _timeout(timeout), _deferExecutionTester(null)
	{
		STINGRAYKIT_CHECK(!_timeout || _timeout >= TimeDuration(), ArgumentException("timeout", _timeout));

		_deferExecutionTester = _deferTaskLifeHolder.GetExecutionTester();
	}


	void ExecutionDeferrer::Cancel()
	{
		MutexLock l(_cancelMutex);

		_deferTaskLifeHolder.Reset();
		_deferredTaskToken.Reset();

		MutexLock l2(_deferExecutionTesterMutex);
		_deferExecutionTester = _deferTaskLifeHolder.GetExecutionTester();
	}


	void ExecutionDeferrer::Defer(const TaskType& task, optional<TimeDuration> overrideTimeout, optional<TimeDuration> interval)
	{
		if (overrideTimeout)
			STINGRAYKIT_CHECK(overrideTimeout >= TimeDuration(), ArgumentException("overrideTimeout", overrideTimeout));
		else
			STINGRAYKIT_CHECK(_timeout, InvalidOperationException());

		_timer.AddTask(Bind(&ExecutionDeferrer::DoDefer, this, task, overrideTimeout ? *overrideTimeout : *_timeout, interval), GetDeferExecutionTester());
	}


	void ExecutionDeferrer::DoDefer(const TaskType& task, TimeDuration timeout, optional<TimeDuration> interval)
	{
		if (interval)
			_deferredTaskToken = _timer.SetTimer(timeout, *interval, task);
		else
			_deferredTaskToken = _timer.SetTimeout(timeout, task);
	}


	ExecutionDeferrerWithTimer::ExecutionDeferrerWithTimer(const std::string& timerName, optional<TimeDuration> timeout)
		: _timer(make_shared_ptr<Timer>(timerName)), _impl(*_timer, timeout)
	{ }

}
