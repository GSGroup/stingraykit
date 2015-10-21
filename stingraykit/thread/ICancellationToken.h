#ifndef STINGRAYKIT_THREAD_ICANCELLATIONTOKEN_H
#define STINGRAYKIT_THREAD_ICANCELLATIONTOKEN_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/time/Time.h>
#include <stingraykit/safe_bool.h>

namespace stingray
{

	struct ICancellationHandler
	{
	protected:
		~ICancellationHandler() { }

	public:
		virtual void Cancel() = 0;
		virtual void Reset() { }
	};


	class CancellationRegistratorBase;


	struct ICancellationToken : public safe_bool<ICancellationToken>
	{
	private:
		friend class CancellationRegistratorBase;

	protected:
		virtual ~ICancellationToken() { }

	public:
		virtual void Cancel() = 0;
		virtual void Reset() = 0;

		virtual void Sleep(TimeDuration duration) const = 0;

		virtual bool IsCancelled() const = 0;

		bool boolean_test() const { return !IsCancelled(); }

	protected:
		virtual bool TryRegisterCancellationHandler(ICancellationHandler& handler) const = 0;
		virtual bool TryUnregisterCancellationHandler() const = 0;
		virtual bool UnregisterCancellationHandler() const = 0;
	};

}

#endif
