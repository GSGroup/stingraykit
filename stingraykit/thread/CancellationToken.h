#ifndef STINGRAYKIT_THREAD_CANCELLATIONTOKEN_H
#define STINGRAYKIT_THREAD_CANCELLATIONTOKEN_H


#include <stingraykit/thread/ConditionVariable.h>
#include <stingraykit/thread/ICancellationToken.h>
#include <stingraykit/thread/DummyCancellationToken.h>
#include <stingraykit/thread/Thread.h>
#include <stingraykit/function/function.h>
#include <stingraykit/optional.h>


namespace stingray
{

	class CancellationToken : public ICancellationToken
	{
		STINGRAYKIT_NONCOPYABLE(CancellationToken);

	private:
		Mutex							_mutex;
		ConditionVariable				_cond;
		bool							_cancelled;
		bool							_cancelDone;
		mutable ICancellationHandler*	_cancelHandler;

	public:
		CancellationToken();
		virtual ~CancellationToken();

		virtual void Cancel();
		virtual void Reset();

		virtual void Sleep(TimeDuration duration) const;

		virtual bool IsCancelled() const;

	protected:
		virtual bool RegisterCancellationHandler(ICancellationHandler& handler) const;
		virtual bool TryUnregisterCancellationHandler() const;
		virtual void UnregisterCancellationHandler() const;
	};

}


#endif
