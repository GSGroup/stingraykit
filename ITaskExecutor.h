#ifndef STINGRAY_TOOLKIT_ITASKEXECUTOR_H
#define STINGRAY_TOOLKIT_ITASKEXECUTOR_H


#include <stingray/toolkit/function.h>
#include <stingray/toolkit/shared_ptr.h>
#include <stingray/toolkit/task_alive_token.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_threads
	 * @{
	 */

	struct ITaskExecutor
	{
		virtual void AddTask(const function<void()>& task) = 0;
		virtual void AddTask(const function<void()>& task, const FutureExecutionTester& tester) = 0;
		virtual ~ITaskExecutor() {}

		static shared_ptr<ITaskExecutor> Create(const std::string& name, const function<void(const std::exception&)>& exceptionHandler = &ITaskExecutor::DefaultExceptionHandler, bool profileCalls = true);

		static void DefaultExceptionHandler(const std::exception& ex);
	};
	TOOLKIT_DECLARE_PTR(ITaskExecutor);

	/** @} */

}


#endif
