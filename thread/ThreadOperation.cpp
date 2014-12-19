#include <stingray/toolkit/thread/ThreadOperation.h>

#include <stingray/toolkit/thread/Thread.h>
#include <stingray/toolkit/diagnostics/Backtrace.h>


namespace stingray
{

	TOOLKIT_DEFINE_NAMED_LOGGER(ThreadOperationReporter);

	ThreadOperationReporter::ThreadOperationReporter(ThreadOperation op)
	{
		if (Thread::GetCurrentThreadName() == "ui")
			s_logger.Error() << op << " operations are prohibited in ui thread!\nBacktrace: " << Backtrace();
	}

	ThreadOperationReporter::~ThreadOperationReporter()
	{ }

}
