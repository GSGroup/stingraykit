#include <stingraykit/thread/CancellationToken.h>


namespace stingray
{

	CancellationToken::CancellationToken() : _cancelled(false), _cancelDone(false), _cancelHandler(null)
	{ }


	CancellationToken::~CancellationToken()
	{ }


	void CancellationToken::Cancel()
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


	void CancellationToken::Reset()
	{
		MutexLock l(_mutex);
		STINGRAYKIT_CHECK(!_cancelHandler && (_cancelled == _cancelDone), LogicException("CancellationToken is in use!"));
		_cancelled = false;
		_cancelDone = false;
	}


	void CancellationToken::Sleep(TimeDuration duration) const
	{
		MutexLock l(_mutex);
		if (_cancelled)
			return;

		_cond.TimedWait(_mutex, duration);
	}


	bool CancellationToken::IsCancelled() const
	{ MutexLock l(_mutex); return _cancelled; }


	bool CancellationToken::RegisterCancellationHandler(ICancellationHandler& handler) const
	{
		MutexLock l(_mutex);
		if (_cancelled)
			return false;

		STINGRAYKIT_CHECK(!_cancelHandler, "Cancellation handler already registered");
		_cancelHandler = &handler;
		return true;
	}


	bool CancellationToken::TryUnregisterCancellationHandler() const
	{
		MutexLock l(_mutex);
		_cancelHandler = null;

		return !_cancelled;
	}


	void CancellationToken::UnregisterCancellationHandler() const
	{
		MutexLock l(_mutex);
		_cancelHandler = null;

		if (!_cancelled)
			return;

		while (!_cancelDone)
			_cond.Wait(_mutex);
	}

}
