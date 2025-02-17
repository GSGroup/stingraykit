#ifndef STINGRAYKIT_THREAD_ICANCELLATIONTOKEN_H
#define STINGRAYKIT_THREAD_ICANCELLATIONTOKEN_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/time/Time.h>

namespace stingray
{

	struct ICancellationHandler
	{
	protected:
		virtual ~ICancellationHandler() { }

	public:
		virtual void Cancel() = 0;
		virtual void Reset() { }
	};


	struct ICancellationToken
	{
		friend class CancellationRegistratorBase;

	protected:
		virtual ~ICancellationToken() { }

	public:
		virtual bool Sleep(optional<TimeDuration> duration) const = 0;

		virtual bool IsCancelled() const = 0;
		virtual bool IsTimedOut() const = 0;

		virtual optional<TimeDuration> GetTimeout() const = 0;

		explicit operator bool () const { return !IsCancelled() && !IsTimedOut(); }

	protected:
		virtual bool TryRegisterCancellationHandler(ICancellationHandler& handler) const = 0;
		virtual bool TryUnregisterCancellationHandler() const = 0;
		virtual bool UnregisterCancellationHandler() const = 0;
	};


#define STINGRAYKIT_CHECK_CANCELLATION(TokenObj) \
		do { \
			STINGRAYKIT_CHECK(!(TokenObj).IsCancelled(), OperationCancelledException()); \
			STINGRAYKIT_CHECK(!(TokenObj).IsTimedOut(), TimeoutException()); \
		} while(false)

#define STINGRAYKIT_ASSUME_CANCELLATION(TokenObj) \
		do { \
			STINGRAYKIT_CHECK_CANCELLATION(TokenObj); \
			STINGRAYKIT_THROW("Abnormal behaviour"); \
		} while(false)

}

#endif
