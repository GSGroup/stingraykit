// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/executor/ExecutionDeferrer.h>

#include <stingraykit/function/bind.h>

namespace stingray
{

	ExecutionDeferrer::ExecutionDeferrer(Timer& timer, optional<TimeDuration> timeout)
		: _timer(timer), _timeout(timeout), _cancellationActive(false)
	{ STINGRAYKIT_CHECK(!_timeout || _timeout >= TimeDuration(), ArgumentException("timeout", _timeout)); }


	void ExecutionDeferrer::Cancel()
	{
		{
			MutexLock l(_mutex);
			if (_cancellationActive)
				return;
			_cancellationActive = true;
		}

		{
			MutexLock l(_doDeferConnectionMutex);
			_doDeferConnection.Reset();
		}
		{
			MutexLock l(_connectionMutex);
			_connection.Reset();
		}

		MutexLock l(_mutex);
		_cancellationActive = false;
	}


	void ExecutionDeferrer::Defer(const TaskType& task)
	{
		STINGRAYKIT_CHECK(_timeout, InvalidOperationException());
		Defer(task, _timeout);
	}


	void ExecutionDeferrer::Defer(const TaskType& task, TimeDuration timeout, optional<TimeDuration> interval)
	{
		MutexLock l(_doDeferConnectionMutex);
		_doDeferConnection = _timer.SetTimeout(TimeDuration(), Bind(&ExecutionDeferrer::DoDefer, this, task, timeout, interval));
	}


	void ExecutionDeferrer::DoDefer(const TaskType& task, TimeDuration timeout, optional<TimeDuration> interval)
	{
		MutexLock l(_connectionMutex);
		if (interval)
			_connection = _timer.SetTimer(timeout, *interval, task);
		else
			_connection = _timer.SetTimeout(timeout, task);
	}

}
