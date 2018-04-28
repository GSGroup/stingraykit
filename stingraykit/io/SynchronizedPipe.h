#ifndef STINGRAYKIT_IO_SYNCHRONIZEDPIPE_H
#define STINGRAYKIT_IO_SYNCHRONIZEDPIPE_H

#include <stingraykit/function/functional.h>
#include <stingraykit/io/IPipe.h>
#include <stingraykit/ScopeExit.h>
#include <stingraykit/thread/ConditionVariable.h>

namespace stingray
{

	class SynchronizedPipe : public virtual IPipe
	{
	private:
		Mutex				_guard;
		ConditionVariable	_cv;
		IPipePtr			_pipe;

	public:
		explicit SynchronizedPipe(const IPipePtr& pipe) : _pipe(STINGRAYKIT_REQUIRE_NOT_NULL(pipe)) { }

		virtual u64 Read(ByteData data, const ICancellationToken& token)
		{ STINGRAYKIT_THROW(NotSupportedException()); }

		virtual u64 Write(ConstByteData data, const ICancellationToken& token)
		{
			MutexLock l(_guard);
			while (!_pipe)
				switch (_cv.Wait(_guard, token))
				{
				case ConditionWaitResult::Broadcasted:	continue;
				case ConditionWaitResult::Cancelled:	return 0;
				case ConditionWaitResult::TimedOut:		STINGRAYKIT_THROW(TimeoutException());
				}

			const IPipePtr pipe = _pipe;
			_pipe = null;

			const ScopeExitInvoker sei1(bind(&ConditionVariable::Broadcast, ref(_cv)));
			const ScopeExitInvoker sei2(bind(make_assigner(_pipe), pipe));

			MutexUnlock ul(l);
			CheckedWriteAll(*pipe, data, token);
			return data.size();
		}
	};

}

#endif
