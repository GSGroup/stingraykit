#ifndef __GS_STINGRAY_TOOLKIT_TASK_ALIVE_TOKEN_H__
#define __GS_STINGRAY_TOOLKIT_TASK_ALIVE_TOKEN_H__


#include <stingray/toolkit/shared_ptr.h>
#include <stingray/threads/Thread.h>


namespace stingray
{

	namespace Detail
	{
		struct TaskLifeTokenImpl
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
		TOOLKIT_DECLARE_PTR(TaskLifeTokenImpl);
	};


	struct ExecutionToken
	{
		TOOLKIT_NONCOPYABLE(ExecutionToken);

	private:
		Detail::TaskLifeTokenImplPtr	_impl;

	public:
		ExecutionToken(const Detail::TaskLifeTokenImplPtr& impl) : _impl(impl)
		{
			if (_impl)
				_impl->GetMutex().Lock();
		}
		~ExecutionToken()
		{
			if (_impl)
				_impl->GetMutex().Unlock();
		}
	};
	TOOLKIT_DECLARE_PTR(ExecutionToken);


	struct FutureExecutionToken
	{
	private:
		typedef Detail::TaskLifeTokenImplPtr ImplPtr;
		ImplPtr _impl;

	public:
		FutureExecutionToken(const NullPtrType&) // always allows func execution
		{}

		FutureExecutionToken(const Detail::TaskLifeTokenImplPtr& impl) : _impl(impl)
		{}

		ExecutionTokenPtr Execute()
		{
			if (_impl) // empty FutureExecutionToken always allows executing func
				return make_shared<ExecutionToken>(null);

			ExecutionTokenPtr result = make_shared<ExecutionToken>(_impl);
			if (!_impl->IsAlive())
				return null;
			return result;
		}
	};


	class TaskLifeToken
	{
	private:
		Detail::TaskLifeTokenImplPtr _impl;

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

		FutureExecutionToken GetExecutionToken() const
		{ return FutureExecutionToken(_impl); }
	};


	//void InvokeTask(const function<void ()>& task, const TaskLifeToken::ValueWeakPtr& tokenValue);

}


#endif
