#include <stingraykit/thread/ITaskExecutor.h>

#include <stingraykit/thread/ThreadTaskExecutor.h>
#include <stingraykit/log/Logger.h>


namespace stingray
{

	ITaskExecutorPtr ITaskExecutor::Create(const std::string& name, const function<void(const std::exception&)>& exceptionHandler, bool profileCalls)
	{ return make_shared<ThreadTaskExecutor>(name, exceptionHandler, profileCalls); }


	void ITaskExecutor::DefaultExceptionHandler(const std::exception& ex)
	{ Logger::Error() << "Uncaught exception in ITaskExecutor: " << ex; }

}
