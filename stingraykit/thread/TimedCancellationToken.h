#ifndef STINGRAYKIT_THREAD_TIMEDCANCELLATIONTOKEN_H
#define STINGRAYKIT_THREAD_TIMEDCANCELLATIONTOKEN_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/CancellationRegistrator.h>
#include <stingraykit/time/RemainingTime.h>

namespace stingray
{

	class TimedCancellationToken : public ICancellationToken
	{
		class ProxyCancellationRegistrator : public CancellationRegistratorBase
		{
		private:
			ICancellationHandler*	_handler;

		public:
			ProxyCancellationRegistrator(const ICancellationToken& token) : CancellationRegistratorBase(token) { }

			bool TryRegisterCancellationHandler(ICancellationHandler& handler)
			{
				Register(handler);
				_handler = &handler;
				return !IsCancelled();
			}

			bool TryUnregisterCancellationHandler()
			{ return TryUnregister(*_handler); }

			bool UnregisterCancellationHandler()
			{
				Unregister(*_handler);
				return !IsCancelled();
			}
		};

	private:
		const ICancellationToken&				_token;
		RemainingTime							_remaining;

		mutable ProxyCancellationRegistrator	_registrator;

	public:
		explicit TimedCancellationToken(TimeDuration timeout);

		TimedCancellationToken(const ICancellationToken& token, TimeDuration timeout);

		virtual ~TimedCancellationToken() { }

		virtual void Sleep(TimeDuration duration) const			{ _token.Sleep(std::min(_remaining.Remaining(), duration)); }

		virtual bool IsCancelled() const						{ return _token.IsCancelled(); }
		virtual bool IsTimedOut() const							{ return _remaining.Expired(); }

		virtual optional<TimeDuration> GetTimeout() const		{ return _remaining.Remaining(); }

	protected:
		virtual bool TryRegisterCancellationHandler(ICancellationHandler& handler) const
		{ return !_remaining.Expired() && _registrator.TryRegisterCancellationHandler(handler); }

		virtual bool TryUnregisterCancellationHandler() const
		{ return _registrator.TryUnregisterCancellationHandler(); }

		virtual bool UnregisterCancellationHandler() const
		{ return _registrator.UnregisterCancellationHandler(); }
	};

}

#endif
