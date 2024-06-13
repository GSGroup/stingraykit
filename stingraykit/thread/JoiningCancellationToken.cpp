// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/JoiningCancellationToken.h>

#include <stingraykit/thread/TimedCancellationToken.h>

namespace stingray
{

	JoiningCancellationToken::JoiningCancellationToken(const ICancellationToken& firstToken, const ICancellationToken& secondToken)
		:	_firstToken(firstToken),
			_secondToken(secondToken),
			_firstRegistrator(_firstToken),
			_firstRegistered(false),
			_secondRegistrator(_secondToken),
			_secondRegistered(false)
	{ }


	bool JoiningCancellationToken::Sleep(optional<TimeDuration> duration) const
	{
		MutexLock l(_mutex);

		if (duration)
			return _cond.Wait(_mutex, TimedCancellationToken(*this, *duration)) == ConditionWaitResult::TimedOut;
		else
		{
			_cond.Wait(_mutex, *this);
			return false;
		}
	}


	optional<TimeDuration> JoiningCancellationToken::GetTimeout() const
	{
		const auto firstTimeout = _firstToken.GetTimeout();
		const auto secondTimeout = _secondToken.GetTimeout();

		return !firstTimeout ? secondTimeout : !secondTimeout ? firstTimeout : std::min(firstTimeout, secondTimeout);
	}


	bool JoiningCancellationToken::TryRegisterCancellationHandler(ICancellationHandler& handler) const
	{
		_firstRegistered = _firstRegistrator.TryRegisterCancellationHandler(handler);
		_secondRegistered = _secondRegistrator.TryRegisterCancellationHandler(handler);

		return _firstRegistered || _secondRegistered;
	}


	bool JoiningCancellationToken::TryUnregisterCancellationHandler() const
	{
		if (_firstRegistered)
			_firstRegistered = !_firstRegistrator.TryUnregisterCancellationHandler();

		if (_secondRegistered)
			_secondRegistered = !_secondRegistrator.TryUnregisterCancellationHandler();

		return !_firstRegistered && !_secondRegistered;
	}


	bool JoiningCancellationToken::UnregisterCancellationHandler() const
	{
		if (_firstRegistered)
		{
			_firstRegistrator.UnregisterCancellationHandler();
			_firstRegistered = false;
		}

		if (_secondRegistered)
		{
			_secondRegistrator.UnregisterCancellationHandler();
			_secondRegistered = false;
		}

		return true;
	}

}
