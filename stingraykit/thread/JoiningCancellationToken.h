#ifndef STINGRAYKIT_THREAD_JOININGCANCELLATIONTOKEN_H
#define STINGRAYKIT_THREAD_JOININGCANCELLATIONTOKEN_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/CancellationRegistrator.h>
#include <stingraykit/thread/ConditionVariable.h>

namespace stingray
{

	/** @brief NOT INTENDED FOR WIDESPREAD USAGE. Use this helper only if another solutions requires much more resources (e.g. additional threads and so on) */
	class JoiningCancellationToken final : public virtual ICancellationToken
	{
		STINGRAYKIT_NONCOPYABLE(JoiningCancellationToken);

	private:
		const ICancellationToken&						_firstToken;
		const ICancellationToken&						_secondToken;

		Mutex											_mutex;
		mutable ConditionVariable						_cond;

		mutable ProxyCancellationRegistrator			_firstRegistrator;
		mutable bool									_firstRegistered;

		mutable ProxyCancellationRegistrator			_secondRegistrator;
		mutable bool									_secondRegistered;

	public:
		JoiningCancellationToken(const ICancellationToken& firstToken, const ICancellationToken& secondToken);

		bool Sleep(optional<TimeDuration> duration) const override;

		bool IsCancelled() const override
		{ return _firstToken.IsCancelled() || _secondToken.IsCancelled(); }

		bool IsTimedOut() const override
		{ return _firstToken.IsTimedOut() || _secondToken.IsTimedOut(); }

		optional<TimeDuration> GetTimeout() const override;

	private:
		bool TryRegisterCancellationHandler(ICancellationHandler& handler) const override;
		bool TryUnregisterCancellationHandler() const override;
		bool UnregisterCancellationHandler() const override;
	};

}

#endif
