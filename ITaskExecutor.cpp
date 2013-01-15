#include <stingray/toolkit/ITaskExecutor.h>

#include <stingray/toolkit/ThreadTaskExecutor.h>
#include <stingray/log/Logger.h>


namespace stingray
{

	ITaskExecutorPtr ITaskExecutor::Create(const std::string& name, const function<void(const std::exception&)>& exceptionHandler)
	{ return make_shared<ThreadTaskExecutor>(name, exceptionHandler); }


	void ITaskExecutor::DefaultExceptionHandler(const std::exception& ex)
	{ Logger::Error() << "Uncaught exception in ITaskExecutor: " << ex; }

}
