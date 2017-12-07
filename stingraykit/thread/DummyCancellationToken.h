#ifndef STINGRAYKIT_THREAD_DUMMYCANCELLATIONTOKEN_H
#define STINGRAYKIT_THREAD_DUMMYCANCELLATIONTOKEN_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/ICancellationToken.h>

namespace stingray
{

	struct DummyCancellationToken : public ICancellationToken
	{
	public:
		virtual bool Sleep(optional<TimeDuration> duration) const;

		virtual bool IsCancelled() const					{ return false; }
		virtual bool IsTimedOut() const 					{ return false; }

		virtual optional<TimeDuration> GetTimeout() const	{ return null; }

	protected:
		virtual bool TryRegisterCancellationHandler(ICancellationHandler& handler) const	{ return true; }
		virtual bool TryUnregisterCancellationHandler() const								{ return true; }
		virtual bool UnregisterCancellationHandler() const									{ return true; }
	};

}

#endif
