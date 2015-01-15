#ifndef STINGRAYKIT_THREAD_CANCELLATIONTOKEN_H
#define STINGRAYKIT_THREAD_CANCELLATIONTOKEN_H


#include <stingraykit/thread/ConditionVariable.h>
#include <stingraykit/thread/ICancellationToken.h>
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
		CancellationToken() : _cancelled(false), _cancelDone(false), _cancelHandler(null)
		{ }

		virtual ~CancellationToken()
		{ }

		virtual void Cancel()
		{
			ICancellationHandler* cancelHandler = null;
			{
				MutexLock l(_mutex);
				if (_cancelled)
					return;

				cancelHandler = _cancelHandler;
				_cancelled = true;
			}

			if (cancelHandler)
				cancelHandler->Cancel();

			{
				MutexLock l(_mutex);
				_cancelDone = true;
				_cond.Broadcast();
			}
		}

		virtual void Reset()
		{
			MutexLock l(_mutex);
			STINGRAYKIT_CHECK(!_cancelHandler && (_cancelled == _cancelDone), LogicException("CancellationToken is in use!"));
			_cancelled = false;
			_cancelDone = false;
		}

		virtual void Sleep(TimeDuration duration) const
		{
			MutexLock l(_mutex);
			if (_cancelled)
				return;

			_cond.TimedWait(_mutex, duration);
		}

		virtual bool IsCancelled() const
		{ MutexLock l(_mutex); return _cancelled; }

	protected:
		virtual bool RegisterCancellationHandler(ICancellationHandler& handler) const
		{
			MutexLock l(_mutex);
			if (_cancelled)
				return false;

			STINGRAYKIT_CHECK(!_cancelHandler, "Cancellation handler already registered");
			_cancelHandler = &handler;
			return true;
		}

		virtual bool TryUnregisterCancellationHandler() const
		{
			MutexLock l(_mutex);
			_cancelHandler = null;

			return !_cancelled;
		}

		virtual void UnregisterCancellationHandler() const
		{
			MutexLock l(_mutex);
			_cancelHandler = null;

			if (!_cancelled)
				return;

			while (!_cancelDone)
				_cond.Wait(_mutex);
		}
	};


	struct DummyCancellationToken : public ICancellationToken
	{
	public:
		virtual void Cancel()							{ }
		virtual void Reset()							{ }

		virtual void Sleep(TimeDuration duration) const	{ Thread::Sleep(duration); }

		virtual bool IsCancelled() const				{ return false; }

	protected:
		virtual bool RegisterCancellationHandler(ICancellationHandler& handler) const	{ return true; }
		virtual bool TryUnregisterCancellationHandler() const							{ return true; }
		virtual void UnregisterCancellationHandler() const								{ }
	};

}


#endif
