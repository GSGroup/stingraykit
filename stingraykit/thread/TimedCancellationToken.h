#ifndef STINGRAYKIT_THREAD_TIMEDCANCELLATIONTOKEN_H
#define STINGRAYKIT_THREAD_TIMEDCANCELLATIONTOKEN_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/CancellationRegistrator.h>
#include <stingraykit/time/RemainingTime.h>

namespace stingray
{

	class TimedCancellationToken final : public virtual ICancellationToken
	{
		STINGRAYKIT_NONCOPYABLE(TimedCancellationToken);

	private:
		const ICancellationToken&				_token;
		RemainingTime							_remaining;

		mutable ProxyCancellationRegistrator	_registrator;

	public:
		explicit TimedCancellationToken(TimeDuration timeout);

		TimedCancellationToken(const ICancellationToken& token, TimeDuration timeout);

		bool Sleep(optional<TimeDuration> duration) const override;

		bool IsCancelled() const override							{ return _token.IsCancelled(); }
		bool IsTimedOut() const override							{ return _remaining.Expired(); }

		optional<TimeDuration> GetTimeout() const override			{ return _remaining.Remaining(); }

	private:
		bool TryRegisterCancellationHandler(ICancellationHandler& handler) const override
		{ return !_remaining.Expired() && _registrator.TryRegisterCancellationHandler(handler); }

		bool TryUnregisterCancellationHandler() const override
		{ return _registrator.TryUnregisterCancellationHandler(); }

		bool UnregisterCancellationHandler() const override
		{ return _registrator.UnregisterCancellationHandler(); }
	};

}

#endif
