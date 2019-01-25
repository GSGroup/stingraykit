#ifndef STINGRAYKIT_THREAD_CANCELLATIONTOKEN_H
#define STINGRAYKIT_THREAD_CANCELLATIONTOKEN_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/ConditionVariable.h>
#include <stingraykit/thread/ICancellationToken.h>
#include <stingraykit/thread/Thread.h>
#include <stingraykit/thread/atomic.h>
#include <stingraykit/Token.h>

namespace stingray
{

	class CancellationToken : public ICancellationToken
	{
		STINGRAYKIT_NONCOPYABLE(CancellationToken);

		class Impl : public self_counter<Impl>
		{
			STINGRAYKIT_NONCOPYABLE(Impl);

			Mutex							_mutex;
			ConditionVariable				_cond;
			atomic<bool>					_cancelled;
			bool							_cancelDone;
			mutable ICancellationHandler*	_cancelHandler;

		public:
			Impl();

			void Cancel();
			void Reset();

			bool Sleep(optional<TimeDuration> duration) const;

			bool IsCancelled() const;

			bool TryRegisterCancellationHandler(ICancellationHandler& handler) const;
			bool TryUnregisterCancellationHandler() const;
			bool UnregisterCancellationHandler() const;
		};
		typedef self_count_ptr<Impl>	ImplPtr;

		ImplPtr							_impl;

	public:
		CancellationToken();

		void Cancel();
		void Reset();

		Token GetCancellator() const;

		virtual bool Sleep(optional<TimeDuration> duration) const;

		virtual bool IsCancelled() const;
		virtual bool IsTimedOut() const { return false; }

		virtual optional<TimeDuration> GetTimeout() const { return null; }

	protected:
		virtual bool TryRegisterCancellationHandler(ICancellationHandler& handler) const;
		virtual bool TryUnregisterCancellationHandler() const;
		virtual bool UnregisterCancellationHandler() const;
	};

}

#endif
