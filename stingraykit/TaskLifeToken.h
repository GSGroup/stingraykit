#ifndef STINGRAYKIT_TASKLIFETOKEN_H
#define STINGRAYKIT_TASKLIFETOKEN_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/thread/Thread.h>
#include <stingraykit/Final.h>
#include <stingraykit/log/Logger.h>
#include <stingraykit/optional.h>
#include <stingraykit/self_counter.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

	namespace Detail
	{

		struct TaskLifeTokenImpl : public self_counter<TaskLifeTokenImpl>
		{
		private:
			Mutex	_sync;
			bool	_alive;
			optional<ThreadId>	_threadId;

		public:
			TaskLifeTokenImpl(bool alive = true) : _alive(alive)
			{ }

			bool TryStartExecution()
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

			void FinishExecution()
			{
				_threadId.reset();
				_sync.Unlock();
			}

			void Kill()
			{
				MutexLock l(_sync);
				if (_threadId && (*_threadId == ThreadEngine::GetCurrentThreadId()))
				{
					std::string backtrace = Backtrace().Get();
					Logger::Error() << "Resetting token while it is locked in current thread!" << (backtrace.empty() ? "" : ("\nbacktrace: " + backtrace));
				}
				_alive = false;
			}
		};
		STINGRAYKIT_DECLARE_SELF_COUNT_PTR(TaskLifeTokenImpl);

	};


	struct LocalExecutionGuard;
	struct FutureExecutionTester
	{
		friend struct LocalExecutionGuard;

	private:
		typedef Detail::TaskLifeTokenImplSelfCountPtr ImplPtr;
		ImplPtr _impl;

	public:
		FutureExecutionTester(const NullPtrType&) // always allows func execution
		{ }

		FutureExecutionTester(const Detail::TaskLifeTokenImplSelfCountPtr& impl) : _impl(impl)
		{ }

		bool IsDummy() const
		{ return !_impl; }
	};


	struct LocalExecutionGuard : public safe_bool<LocalExecutionGuard>
	{
		STINGRAYKIT_NONCOPYABLE(LocalExecutionGuard);

	private:
		bool									_allow;
		Detail::TaskLifeTokenImplSelfCountPtr	_impl;

	public:
		LocalExecutionGuard(const FutureExecutionTester& tester) : _allow(true), _impl(tester._impl)
		{
			if (!_impl)
				return;
			_allow = _impl->TryStartExecution();
			if (!_allow)
				_impl.reset();
		}

		~LocalExecutionGuard()
		{
			if (_impl)
				_impl->FinishExecution();
		}

		bool boolean_test() const
		{ return _allow; }
	};


	class TaskLifeToken : STINGRAYKIT_FINAL(TaskLifeToken)
	{
	private:
		Detail::TaskLifeTokenImplSelfCountPtr _impl;

	public:
		TaskLifeToken() : _impl(new Detail::TaskLifeTokenImpl)
		{ }

		~TaskLifeToken()
		{ }

		void Release()
		{
			if (_impl)
				_impl->Kill();
		}

		TaskLifeToken& Reset()
		{
			Release();
			return (*this = TaskLifeToken());
		}

		bool IsDummy() const
		{ return !_impl; }

		FutureExecutionTester GetExecutionTester() const
		{ return FutureExecutionTester(_impl); }

		static TaskLifeToken CreateDummyTaskToken()
		{ return TaskLifeToken(null); }

		static TaskLifeToken CreateDeadTaskToken()
		{ return TaskLifeToken(make_self_count_ptr<Detail::TaskLifeTokenImpl>(false)); }

	private:
		TaskLifeToken(const Detail::TaskLifeTokenImplSelfCountPtr& impl) : _impl(impl)
		{ }
	};
	STINGRAYKIT_DECLARE_PTR(TaskLifeToken);


	class TaskLifeHolder
	{
		STINGRAYKIT_NONCOPYABLE(TaskLifeHolder);

	private:
		Detail::TaskLifeTokenImplSelfCountPtr _impl;

	public:
		TaskLifeHolder() : _impl(new Detail::TaskLifeTokenImpl()) { }

		~TaskLifeHolder()
		{ Release(); }

		void Release()
		{ _impl->Kill(); }

		TaskLifeHolder& Reset()
		{
			Release();
			_impl.reset(new Detail::TaskLifeTokenImpl());
			return *this;
		}

		FutureExecutionTester GetExecutionTester() const
		{ return FutureExecutionTester(_impl); }
	};

	/** @} */

}


#endif
