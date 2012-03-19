#ifndef __GS_DVRLIB_TOOLKIT_ITASKEXECUTOR_H__
#define __GS_DVRLIB_TOOLKIT_ITASKEXECUTOR_H__


#include <dvrlib/toolkit/function.h>
#include <dvrlib/toolkit/shared_ptr.h>


namespace dvrlib
{


	struct ITaskExecutor
	{
		virtual void AddTask(const function<void()>& task) = 0;
		virtual ~ITaskExecutor() {}

		static shared_ptr<ITaskExecutor> Create(const std::string& name, const function<void(const std::exception&)>& exceptionHandler = &ITaskExecutor::DefaultExceptionHandler);

	private:
		static void DefaultExceptionHandler(const std::exception& ex);
	};
	TOOLKIT_DECLARE_PTR(ITaskExecutor);


}


#endif
