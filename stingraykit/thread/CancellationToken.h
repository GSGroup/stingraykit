#ifndef STINGRAYKIT_THREAD_CANCELLATIONTOKEN_H
#define STINGRAYKIT_THREAD_CANCELLATIONTOKEN_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/atomic.h>
#include <stingraykit/thread/ConditionVariable.h>
#include <stingraykit/thread/ICancellationToken.h>
#include <stingraykit/Token.h>

namespace stingray
{

	class CancellationToken : public virtual ICancellationToken
	{
		STINGRAYKIT_NONCOPYABLE(CancellationToken);

	private:
		class Impl : public self_counter<Impl>
		{
			STINGRAYKIT_NONCOPYABLE(Impl);

		private:
			Mutex							_mutex;
			ConditionVariable				_cond;
			atomic<bool>					_cancelled;
			bool							_cancelDone;
			mutable ICancellationHandler*	_cancelHandler;

		public:
			Impl();

			bool Sleep(optional<TimeDuration> duration) const;

			bool IsCancelled() const;

			void Cancel();
			void Reset();

			bool TryRegisterCancellationHandler(ICancellationHandler& handler) const;
			bool TryUnregisterCancellationHandler() const;
			bool UnregisterCancellationHandler() const;
		};

	private:
		self_count_ptr<Impl>				_impl;

	public:
		CancellationToken();

		bool Sleep(optional<TimeDuration> duration) const override;

		bool IsCancelled() const override;
		bool IsTimedOut() const override { return false; }

		optional<TimeDuration> GetTimeout() const override { return null; }

		void Cancel();
		void Reset();

		Token GetCancellator();

	protected:
		bool TryRegisterCancellationHandler(ICancellationHandler& handler) const override;
		bool TryUnregisterCancellationHandler() const override;
		bool UnregisterCancellationHandler() const override;
	};

}

#endif
