#ifndef STINGRAYKIT_THREAD_DUMMYCANCELLATIONTOKEN_H
#define STINGRAYKIT_THREAD_DUMMYCANCELLATIONTOKEN_H


#include <stingraykit/thread/ICancellationToken.h>

namespace stingray
{

	struct DummyCancellationToken : public ICancellationToken
	{
	public:
		virtual void Cancel()							{ }
		virtual void Reset()							{ }

		virtual void Sleep(TimeDuration duration) const;

		virtual bool IsCancelled() const				{ return false; }

	protected:
		virtual bool RegisterCancellationHandler(ICancellationHandler& handler) const	{ return true; }
		virtual bool TryUnregisterCancellationHandler() const							{ return true; }
		virtual void UnregisterCancellationHandler() const								{ }
	};

}

#endif
