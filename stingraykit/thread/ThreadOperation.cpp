#include <stingraykit/thread/ThreadOperation.h>

#include <stingraykit/thread/Thread.h>
#include <stingraykit/diagnostics/Backtrace.h>


namespace stingray
{

	STINGRAYKIT_DEFINE_NAMED_LOGGER(ThreadOperationReporter);

	ThreadOperationReporter::ThreadOperationReporter(ThreadOperation op)
	{
		if (Thread::GetCurrentThreadName() == "ui")
			s_logger.Error() << op << " operations are prohibited in ui thread!\nBacktrace: " << Backtrace();
	}

	ThreadOperationReporter::~ThreadOperationReporter()
	{ }

}
