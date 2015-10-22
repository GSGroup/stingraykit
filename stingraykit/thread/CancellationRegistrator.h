#ifndef STINGRAYKIT_THREAD_CANCELLATIONREGISTRATOR_H
#define STINGRAYKIT_THREAD_CANCELLATIONREGISTRATOR_H

#include <stingraykit/assert.h>
#include <stingraykit/thread/ICancellationToken.h>

namespace stingray
{

	class CancellationRegistratorBase
	{
	private:
		const ICancellationToken&	_token;
		bool						_registered;

	public:
		bool IsCancelled() const
		{ return !_registered; }

	protected:
		CancellationRegistratorBase(const ICancellationToken& token) :
			_token(token), _registered(false)
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


	class CancellationRegistrator : public CancellationRegistratorBase
	{
		STINGRAYKIT_NONCOPYABLE(CancellationRegistrator);

	private:
		ICancellationHandler& _handler;

	public:
		CancellationRegistrator(ICancellationHandler& handler, const ICancellationToken& token) :
			CancellationRegistratorBase(token), _handler(handler)
		{ Register(_handler); }

		~CancellationRegistrator()
		{
			if (STINGRAYKIT_LIKELY(TryUnregister(_handler)))
				return;
			Unregister(_handler);
		}
	};

}

#endif
