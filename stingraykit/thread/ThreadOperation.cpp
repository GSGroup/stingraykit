#include <stingraykit/thread/ThreadOperation.h>

#include <stingraykit/diagnostics/Backtrace.h>
#include <stingraykit/thread/Thread.h>
#include <stingraykit/thread/posix/ThreadLocal.h>

namespace stingray
{

	STINGRAYKIT_DECLARE_THREAD_LOCAL_POD(int, RestrictedThreadOperations);
	STINGRAYKIT_DEFINE_THREAD_LOCAL_POD(int, RestrictedThreadOperations, 0);

	ThreadOperationConstrainer::ThreadOperationConstrainer(ThreadOperation restrictedOperations) :
		_oldValue(RestrictedThreadOperations::Get())
	{ RestrictedThreadOperations::Set(_oldValue | restrictedOperations.val()); }


	ThreadOperationConstrainer::~ThreadOperationConstrainer()
	{ RestrictedThreadOperations::Set(_oldValue); }


	STINGRAYKIT_DEFINE_NAMED_LOGGER(ThreadOperationReporter);

	ThreadOperationReporter::ThreadOperationReporter(ThreadOperation op)
	{
		if (op.val() & RestrictedThreadOperations::Get())
			s_logger.Error() << op << " operations are prohibited in this thread!\nBacktrace: " << Backtrace();
	}


	ThreadOperationReporter::~ThreadOperationReporter()
	{ }

}
