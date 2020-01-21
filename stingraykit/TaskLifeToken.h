#ifndef STINGRAYKIT_TASKLIFETOKEN_H
#define STINGRAYKIT_TASKLIFETOKEN_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/Thread.h>
#include <stingraykit/Final.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

	namespace Detail
	{

		class TaskLifeTokenImpl : public self_counter<TaskLifeTokenImpl>
		{
		private:
			Mutex				_sync;
			bool				_alive;
			optional<ThreadId>	_threadId;

		public:
			TaskLifeTokenImpl(bool alive = true)
				: _alive(alive)
			{ }

			bool TryStartExecution();
			void FinishExecution();

			void Kill();
		};
		STINGRAYKIT_DECLARE_SELF_COUNT_PTR(TaskLifeTokenImpl);

	};


	class FutureExecutionTester
	{
		friend class LocalExecutionGuard;

	private:
		Detail::TaskLifeTokenImplSelfCountPtr	_impl;

	public:
		FutureExecutionTester(const NullPtrType&) // always allows func execution
		{ }

		FutureExecutionTester(const Detail::TaskLifeTokenImplSelfCountPtr& impl)
			: _impl(impl)
		{ }

		bool IsDummy() const
		{ return !_impl; }
	};


	class LocalExecutionGuard : public safe_bool<LocalExecutionGuard>
	{
		STINGRAYKIT_NONCOPYABLE(LocalExecutionGuard);

	private:
		bool									_allow;
		Detail::TaskLifeTokenImplSelfCountPtr	_impl;

	public:
		LocalExecutionGuard(const FutureExecutionTester& tester);
		~LocalExecutionGuard();

		bool boolean_test() const
		{ return _allow; }
	};


	class TaskLifeToken : STINGRAYKIT_FINAL(TaskLifeToken)
	{
	private:
		Detail::TaskLifeTokenImplSelfCountPtr _impl;

	public:
		TaskLifeToken()
			: _impl(make_self_count_ptr<Detail::TaskLifeTokenImpl>())
		{ }

		void Release();
		TaskLifeToken& Reset();

		bool IsDummy() const
		{ return !_impl; }

		FutureExecutionTester GetExecutionTester() const
		{ return FutureExecutionTester(_impl); }

		static TaskLifeToken CreateDummyTaskToken()
		{ return TaskLifeToken(null); }

		static TaskLifeToken CreateDeadTaskToken()
		{ return TaskLifeToken(make_self_count_ptr<Detail::TaskLifeTokenImpl>(false)); }

	private:
		TaskLifeToken(const Detail::TaskLifeTokenImplSelfCountPtr& impl)
			: _impl(impl)
		{ }
	};


	class TaskLifeHolder
	{
		STINGRAYKIT_NONCOPYABLE(TaskLifeHolder);

	private:
		Detail::TaskLifeTokenImplSelfCountPtr _impl;

	public:
		TaskLifeHolder()
			: _impl(make_self_count_ptr<Detail::TaskLifeTokenImpl>())
		{ }

		~TaskLifeHolder()
		{ Release(); }

		void Release();
		TaskLifeHolder& Reset();

		FutureExecutionTester GetExecutionTester() const
		{ return FutureExecutionTester(_impl); }
	};

	/** @} */

}

#endif
