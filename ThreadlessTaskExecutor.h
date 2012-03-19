#ifndef __GS_DVRLIB_TOOLKIT_THREADLESSTASKEXECUTOR_H__
#define __GS_DVRLIB_TOOLKIT_THREADLESSTASKEXECUTOR_H__


#include <queue>
#include <list>

#include <dvrlib/toolkit/Final.h>
#include <dvrlib/toolkit/ITaskExecutor.h>
#include <dvrlib/threads/Thread.h>


namespace dvrlib
{

	
	class ThreadlessTaskExecutor : TOOLKIT_FINAL(ThreadlessTaskExecutor), public virtual ITaskExecutor
	{
		typedef function<void()>							TaskType;
		typedef function<void(const std::exception&)>		ExceptionHandlerType;
		typedef std::queue<TaskType, std::list<TaskType> >	QueueType;

	private:
		Mutex					_syncRoot;
		QueueType				_queue;
		ExceptionHandlerType	_exceptionHandler;
		

	public:
		explicit ThreadlessTaskExecutor(const ExceptionHandlerType& exceptionHandler)
			: _exceptionHandler(exceptionHandler)
		{ }

		virtual void AddTask(const TaskType& task)
		{
			MutexLock l(_syncRoot);
			_queue.push(task);
		}

		virtual void Pause(bool pause) { TOOLKIT_THROW(NotSupportedException()); }

		void ExecuteTasks()
		{
			MutexLock l(_syncRoot);
			while (!_queue.empty())
			{
				TaskType top = _queue.front();
				_queue.pop();
				try
				{
					MutexUnlock ul(l);
					//Tracer tracer("ThreadlessTaskExecutor::ExecuteTasks: executing pending task"); //fixme: dependency to log/
					top();
					Thread::InterruptionPoint();
				}
				catch(const std::exception& ex)
				{ _exceptionHandler(ex); }
			}
		}
	};

	TOOLKIT_DECLARE_PTR(ThreadlessTaskExecutor);

}



#endif
