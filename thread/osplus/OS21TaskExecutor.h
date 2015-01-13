#ifndef STINGRAYKIT_THREAD_OSPLUS_OS21TASKEXECUTOR_H
#define STINGRAYKIT_THREAD_OSPLUS_OS21TASKEXECUTOR_H

#include <os21/message.h>

#include <string>

#include <stingray/toolkit/thread/Thread.h>
#include <stingray/toolkit/Final.h>
#include <stingray/toolkit/thread/ITaskExecutor.h>


namespace stingray {
namespace osplus
{


	class TaskExecutor
		: STINGRAYKIT_FINAL(TaskExecutor)
		, public virtual ITaskExecutor
	{
		typedef function<void()>							TaskType;
		typedef function<void(const std::exception&)>		ExceptionHandlerType;
		typedef std::pair<TaskType, FutureExecutionTester>	TaskPair;

	private:
		ThreadPtr				_worker;
		ExceptionHandlerType	_exceptionHandler;
		message_queue_t*		_queue;

	public:
		explicit TaskExecutor(const std::string& name, const ExceptionHandlerType& exceptionHandler);
		~TaskExecutor();

		virtual void AddTask(const TaskType& task);
		virtual void AddTask(const TaskType& task, const FutureExecutionTester& tester);

	private:
		void ThreadFunc();
	};


}}


#endif
