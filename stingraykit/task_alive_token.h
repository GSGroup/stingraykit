#ifndef STINGRAYKIT_TASK_ALIVE_TOKEN_H
#define STINGRAYKIT_TASK_ALIVE_TOKEN_H


#include <stingraykit/thread/Thread.h>
#include <stingraykit/Final.h>
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

		public:
			TaskLifeTokenImpl() : _alive(true)
			{}
			Mutex& GetMutex() 		{ return _sync; }
			bool IsAlive() const	{ return _alive; }
			void Kill()				{ _alive = false; }
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

			_impl->GetMutex().Lock();
			_allow = _impl->IsAlive();
			if (!_allow)
			{
				_impl->GetMutex().Unlock();
				_impl.reset();
			}
		}

		~LocalExecutionGuard()
		{
			if (_impl)
				_impl->GetMutex().Unlock();
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
		{}

		~TaskLifeToken()
		{}

		void Release()
		{
			MutexLock l(_impl->GetMutex());
			_impl->Kill();
		}

		TaskLifeToken& Reset()
		{
			Release();
			return (*this = TaskLifeToken());
		}

		FutureExecutionTester GetExecutionTester() const
		{ return FutureExecutionTester(_impl); }
	};
	STINGRAYKIT_DECLARE_PTR(TaskLifeToken);

	/** @} */

}


#endif
