// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#ifndef STINGRAYKIT_THREAD_CANCELLATIONREGISTRATOR_H
#define STINGRAYKIT_THREAD_CANCELLATIONREGISTRATOR_H

#include <stingraykit/assert.h>
#include <stingraykit/thread/ICancellationToken.h>

namespace stingray
{

	class CancellationRegistratorBase
	{
		STINGRAYKIT_NONCOPYABLE(CancellationRegistratorBase);

	private:
		const ICancellationToken&	_token;
		bool						_registered;

	public:
		bool IsCancelled() const
		{ return !_registered; }

	protected:
		CancellationRegistratorBase(const ICancellationToken& token)
			: _token(token), _registered(false)
		{ }

		~CancellationRegistratorBase()
		{ STINGRAYKIT_ASSERT(!_registered); }

		void Register(ICancellationHandler& handler)
		{ _registered = _token.TryRegisterCancellationHandler(handler); }

		bool TryUnregister(ICancellationHandler& handler)
		{
			if (!_registered)
				return true;

			_registered = !_token.TryUnregisterCancellationHandler();
			return !_registered;
		}

		void Unregister(ICancellationHandler& handler)
		{
			if (!_registered)
				return;

			if (!_token.UnregisterCancellationHandler())
				handler.Reset();
			_registered = false;
		}
	};


	class CancellationRegistrator final : public CancellationRegistratorBase
	{
	private:
		ICancellationHandler&		_handler;

	public:
		CancellationRegistrator(const ICancellationToken& token, ICancellationHandler& handler)
			: CancellationRegistratorBase(token), _handler(handler)
		{ Register(_handler); }

		~CancellationRegistrator()
		{
			if (STINGRAYKIT_LIKELY(TryUnregister(_handler)))
				return;
			Unregister(_handler);
		}
	};


	class ProxyCancellationRegistrator final : public CancellationRegistratorBase
	{
	private:
		ICancellationHandler*		_handler;

	public:
		ProxyCancellationRegistrator(const ICancellationToken& token)
			: CancellationRegistratorBase(token)
		{ }

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
			return true;
		}
	};

}

#endif
