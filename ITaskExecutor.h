#ifndef __GS_DVRLIB_TOOLKIT_ITASKEXECUTOR_H__
#define __GS_DVRLIB_TOOLKIT_ITASKEXECUTOR_H__


#include <stingray/toolkit/function.h>
#include <stingray/toolkit/shared_ptr.h>
#include <stingray/toolkit/task_alive_token.h>
#include <stingray/threads/Thread.h>


namespace stingray
{

	struct ITaskExecutor
	{
		virtual void AddTask(const function<void()>& task) = 0;
		virtual void AddTask(const function<void()>& task, const FutureExecutionToken& token) = 0;
		virtual ~ITaskExecutor() {}

		static shared_ptr<ITaskExecutor> Create(const std::string& name, const function<void(const std::exception&)>& exceptionHandler = &ITaskExecutor::DefaultExceptionHandler);

	private:
		static void DefaultExceptionHandler(const std::exception& ex);
	};
	TOOLKIT_DECLARE_PTR(ITaskExecutor);


}


#endif
