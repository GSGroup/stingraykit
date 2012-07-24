#ifndef __GS_STINGRAY_TOOLKIT_TASK_ALIVE_TOKEN_H__
#define __GS_STINGRAY_TOOLKIT_TASK_ALIVE_TOKEN_H__


#include <stingray/toolkit/shared_ptr.h>
#include <stingray/threads/Thread.h>


namespace stingray
{

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
		TOOLKIT_DECLARE_SELF_COUNT_PTR(TaskLifeTokenImpl);
	};


	struct FutureExecutionToken;
	struct ExecutionToken : public safe_bool<ExecutionToken>
	{
		TOOLKIT_NONCOPYABLE(ExecutionToken);

	private:
		bool									_allow;
		Detail::TaskLifeTokenImplSelfCountPtr	_impl;

	public:
		ExecutionToken() : _allow(false)
		{}
		~ExecutionToken()
		{
			if (_impl)
				_impl->GetMutex().Unlock();
		}
		bool boolean_test() const	{ return _allow; }

	private:
		friend struct FutureExecutionToken;
		void SetImpl(const Detail::TaskLifeTokenImplSelfCountPtr& impl)
		{
			if (!impl)
			{
				_allow = true;
				return;
			}
			_impl = impl;
			_impl->GetMutex().Lock();
			_allow = _impl->IsAlive();
			if (_allow)
				return;
			_impl->GetMutex().Unlock();
			_impl.reset();
		}
	};


	struct FutureExecutionToken
	{
	private:
		typedef Detail::TaskLifeTokenImplSelfCountPtr ImplPtr;
		ImplPtr _impl;

	public:
		FutureExecutionToken(const NullPtrType&) // always allows func execution
		{}

		FutureExecutionToken(const Detail::TaskLifeTokenImplSelfCountPtr& impl) : _impl(impl)
		{}

		ExecutionToken& Execute(ExecutionToken& token) { token.SetImpl(null); return token; }
	};


	class TaskLifeToken
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

		FutureExecutionToken GetExecutionToken() const
		{ return FutureExecutionToken(_impl); }
	};


	//void InvokeTask(const function<void ()>& task, const TaskLifeToken::ValueWeakPtr& tokenValue);

}


#endif
