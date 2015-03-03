#include <stingraykit/thread/ThreadOperation.h>

#include <stingraykit/diagnostics/Backtrace.h>
#include <stingraykit/thread/Thread.h>
#include <stingraykit/thread/posix/ThreadLocal.h>

namespace stingray
{

	STINGRAYKIT_DECLARE_THREAD_LOCAL(int, RestrictedThreadOperations);
	STINGRAYKIT_DEFINE_THREAD_LOCAL(int, RestrictedThreadOperations);

	ThreadOperationConstrainer::ThreadOperationConstrainer(ThreadOperation restrictedOperations) :
		_oldValue(RestrictedThreadOperations::Get())
	{ RestrictedThreadOperations::Get() = _oldValue | restrictedOperations.val(); }


	ThreadOperationConstrainer::~ThreadOperationConstrainer()
	{ RestrictedThreadOperations::Get() = _oldValue; }


	STINGRAYKIT_DEFINE_NAMED_LOGGER(ThreadOperationReporter);

	ThreadOperationReporter::ThreadOperationReporter(ThreadOperation op)
	{
		if (op.val() & RestrictedThreadOperations::Get())
			s_logger.Error() << op << " operations are prohibited in this thread!\nBacktrace: " << Backtrace();
	}


	ThreadOperationReporter::~ThreadOperationReporter()
	{ }

}
