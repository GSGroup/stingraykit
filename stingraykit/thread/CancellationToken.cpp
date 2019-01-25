// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/CancellationToken.h>

#include <stingraykit/function/bind.h>
#include <stingraykit/FunctionToken.h>

namespace stingray
{

	CancellationToken::Impl::Impl()
		:	_cancelled(false),
			_cancelDone(false),
			_cancelHandler(null)
	{ }


	void CancellationToken::Impl::Cancel()
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


	void CancellationToken::Impl::Reset()
	{
		MutexLock l(_mutex);
		STINGRAYKIT_CHECK(!_cancelHandler && (_cancelled == _cancelDone), LogicException("CancellationToken is in use!"));
		_cancelled = false;
		_cancelDone = false;
	}


	bool CancellationToken::Impl::Sleep(optional<TimeDuration> duration) const
	{
		MutexLock l(_mutex);
		if (_cancelled)
			return false;

		if (duration)
			return !_cond.TimedWait(_mutex, *duration);

		_cond.Wait(_mutex);
		return false;
	}


	bool CancellationToken::Impl::IsCancelled() const
	{ return _cancelled.load(MemoryOrderRelaxed); }


	bool CancellationToken::Impl::TryRegisterCancellationHandler(ICancellationHandler& handler) const
	{
		MutexLock l(_mutex);
		if (_cancelled)
			return false;

		STINGRAYKIT_CHECK(!_cancelHandler, "Cancellation handler already registered");
		_cancelHandler = &handler;
		return true;
	}


	bool CancellationToken::Impl::TryUnregisterCancellationHandler() const
	{
		MutexLock l(_mutex);
		_cancelHandler = null;

		return !_cancelled;
	}


	bool CancellationToken::Impl::UnregisterCancellationHandler() const
	{
		MutexLock l(_mutex);
		_cancelHandler = null;

		if (!_cancelled)
			return true;

		while (!_cancelDone)
			_cond.Wait(_mutex);
		return false;
	}


	CancellationToken::CancellationToken()
		:	_impl(new Impl())
	{ }


	void CancellationToken::Cancel()
	{ return _impl->Cancel(); }


	void CancellationToken::Reset()
	{ _impl->Reset(); }


	Token CancellationToken::GetCancellator() const
	{ return MakeToken<FunctionToken>(bind(&Impl::Cancel, _impl)); }


	bool CancellationToken::Sleep(optional<TimeDuration> duration) const
	{ return _impl->Sleep(duration); }


	bool CancellationToken::IsCancelled() const
	{ return _impl->IsCancelled(); }


	bool CancellationToken::TryRegisterCancellationHandler(ICancellationHandler& handler) const
	{ return _impl->TryRegisterCancellationHandler(handler); }


	bool CancellationToken::TryUnregisterCancellationHandler() const
	{ return _impl->TryUnregisterCancellationHandler(); }


	bool CancellationToken::UnregisterCancellationHandler() const
	{ return _impl->UnregisterCancellationHandler(); }

}
