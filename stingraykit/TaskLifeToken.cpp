// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/TaskLifeToken.h>
#include <stingraykit/log/Logger.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

	namespace Detail
	{

		void TaskLifeTokenImpl::NotifyDestruction() const
		{
			if (_alive.load(MemoryOrderRelaxed))
				Logger::Error() << "[TaskLifeToken] Destroying unreleased token\nbacktrace: " << Backtrace();
		}


		bool TaskLifeTokenImpl::TryStartExecution()
		{
			_sync.Lock();

			if (_alive)
			{
				_threadId = ThreadEngine::GetCurrentThreadId();
				return true;
			}

			_sync.Unlock();
			return false;
		}


		void TaskLifeTokenImpl::FinishExecution()
		{
			_threadId.reset();
			_sync.Unlock();
		}


		void TaskLifeTokenImpl::Kill()
		{
			MutexLock l(_sync);

			if (_threadId && _threadId == ThreadEngine::GetCurrentThreadId())
				Logger::Error() << "[TaskLifeToken] Resetting token locked in current thread\nbacktrace: " << Backtrace();

			_alive = false;
		}

	};


	LocalExecutionGuard::LocalExecutionGuard(const FutureExecutionTester& tester)
		: _allow(true), _impl(tester._impl)
	{
		if (!_impl)
			return;
		_allow = _impl->TryStartExecution();
		if (!_allow)
			_impl.reset();
	}


	LocalExecutionGuard::~LocalExecutionGuard()
	{
		if (_impl)
			_impl->FinishExecution();
	}


	TaskLifeToken::~TaskLifeToken()
	{
		if (_impl)
			_impl->NotifyDestruction();
	}


	void TaskLifeToken::Release()
	{
		if (_impl)
		{
			_impl->Kill();
			_impl.reset();
		}
	}


	TaskLifeToken& TaskLifeToken::Reset()
	{
		Release();
		return (*this = TaskLifeToken());
	}


	void TaskLifeHolder::Release()
	{
		if (_impl)
		{
			_impl->Kill();
			_impl.reset();
		}
	}


	TaskLifeHolder& TaskLifeHolder::Reset()
	{
		Release();
		_impl = make_self_count_ptr<Detail::TaskLifeTokenImpl>();
		return *this;
	}

	/** @} */

}
