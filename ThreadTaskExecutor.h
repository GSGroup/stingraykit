#ifndef __GS_DVRLIB_TOOLKIT_THREADTASKEXECUTOR_H__
#define __GS_DVRLIB_TOOLKIT_THREADTASKEXECUTOR_H__

#include <queue>
#include <list>

#include <dvrlib/threads/ConditionVariable.h>
#include <dvrlib/threads/Thread.h>
#include <dvrlib/toolkit/Final.h>
#include <dvrlib/toolkit/ITaskExecutor.h>


namespace dvrlib
{

	
	class ThreadTaskExecutor : TOOLKIT_FINAL(ThreadTaskExecutor), public virtual ITaskExecutor
	{
		typedef function<void()>							TaskType;
		typedef function<void(const std::exception&)>		ExceptionHandlerType;
		typedef std::queue<TaskType, std::list<TaskType> >	QueueType;

	private:
		volatile bool			_working;
		volatile bool			_paused;
		ThreadPtr				_worker;
		Mutex					_syncRoot;
		ConditionVariable		_condVar;
		QueueType				_queue;
		ExceptionHandlerType	_exceptionHandler;
		

	public:
		explicit ThreadTaskExecutor(const std::string& name, const ExceptionHandlerType& exceptionHandler);
		~ThreadTaskExecutor();

		virtual void AddTask(const TaskType& task);
		virtual void Pause(bool pause);

	private:
		void ThreadFunc();
	};
	TOOLKIT_DECLARE_PTR(ThreadTaskExecutor);

}


#endif
