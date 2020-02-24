#ifndef STINGRAYKIT_IO_SYNCHRONIZEDPIPE_H
#define STINGRAYKIT_IO_SYNCHRONIZEDPIPE_H

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

			IPipePtr pipe;
			_pipe.swap(pipe);

			const ScopeExitInvoker sei1(Bind(&ConditionVariable::Broadcast, wrap_ref(_cv)));
			const ScopeExitInvoker sei2(Bind(&IPipePtr::swap, wrap_ref(_pipe), wrap_ref(pipe)));

			MutexUnlock ul(l);
			CheckedWriteAll(*pipe, data, token);
			return data.size();
		}

		virtual bool Peek(const ICancellationToken& token)
		{ STINGRAYKIT_THROW(NotSupportedException()); }
	};

}

#endif
