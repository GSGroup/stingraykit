// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

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
	{ return _cancelled.load(MemoryOrderRelaxed); }


	bool CancellationToken::TryRegisterCancellationHandler(ICancellationHandler& handler) const
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


	bool CancellationToken::UnregisterCancellationHandler() const
	{
		MutexLock l(_mutex);
		_cancelHandler = null;

		if (!_cancelled)
			return true;

		while (!_cancelDone)
			_cond.Wait(_mutex);
		return false;
	}

}
