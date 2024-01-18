// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#ifndef STINGRAYKIT_IO_SYNCHRONIZEDPIPE_H
#define STINGRAYKIT_IO_SYNCHRONIZEDPIPE_H

#include <stingraykit/function/bind.h>
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
				case ConditionWaitResult::Cancelled:	STINGRAYKIT_THROW(OperationCancelledException());
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
