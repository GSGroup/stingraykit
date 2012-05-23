#ifndef __GS_DVRLIB_TOOLKIT_THREADLESSTASKEXECUTOR_H__
#define __GS_DVRLIB_TOOLKIT_THREADLESSTASKEXECUTOR_H__


#include <queue>
#include <list>

#include <stingray/toolkit/Final.h>
#include <stingray/toolkit/ITaskExecutor.h>
#include <stingray/threads/Thread.h>


namespace stingray
{

	
	class ThreadlessTaskExecutor : TOOLKIT_FINAL(ThreadlessTaskExecutor), public virtual ITaskExecutor
	{
		typedef function<void()>										TaskType;
		typedef function<void(const std::exception&)>					ExceptionHandlerType;
		typedef std::pair<TaskType, task_alive_token::ValueWeakPtr>		TaskPair;
		typedef std::queue<TaskPair, std::list<TaskPair> >				QueueType;

	private:
		Mutex					_syncRoot;
		QueueType				_queue;
		ExceptionHandlerType	_exceptionHandler;
		task_alive_token		_token;
		

	public:
		explicit ThreadlessTaskExecutor(const ExceptionHandlerType& exceptionHandler)
			: _exceptionHandler(exceptionHandler)
		{ }

		virtual void AddTask(const TaskType& task, const task_alive_token& token)
		{
			MutexLock l(_syncRoot);
			_queue.push(std::make_pair(task, token.get_value()));
		}

		virtual void AddTask(const TaskType& task)
		{ AddTask(task, _token); }

		virtual void Pause(bool pause) { TOOLKIT_THROW(NotSupportedException()); }

		void ExecuteTasks()
		{
			MutexLock l(_syncRoot);
			while (!_queue.empty())
			{
				TaskPair top = _queue.front();
				_queue.pop();
				try
				{
					MutexUnlock ul(l);
					//Tracer tracer("ThreadlessTaskExecutor::ExecuteTasks: executing pending task"); //fixme: dependency to log/
					InvokeTask(top.first, top.second);
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
