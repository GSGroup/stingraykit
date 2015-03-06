// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/posix/PosixConditionVariable.h>

#include <errno.h>

#include <stingraykit/time/posix/TimeEngine.h>
#include <stingraykit/time/posix/utils.h>
#include <stingraykit/ScopeExit.h>
#include <stingraykit/Singleton.h>
#include <stingraykit/reference.h>

namespace stingray
{

	struct CancellationHolder : public CancellationHandlerHolderBase
	{
		struct CancellationHandler : public ICancellationHandler
		{
		private:
			PosixMutex&				_mutex;
			PosixConditionVariable&	_cond;

		public:
			CancellationHandler(PosixMutex& mutex, PosixConditionVariable& cond) : _mutex(mutex), _cond(cond)
			{ }

			virtual ~CancellationHandler()
			{ }

			virtual void Cancel()
			{
				MutexLock l(_mutex);
				_cond.Broadcast();
			}

			PosixMutex& GetMutex() const
			{ return _mutex; }
		};

	private:
		CancellationHandler	_handler;

	public:
		CancellationHolder(PosixMutex& mutex, PosixConditionVariable& cond, const ICancellationToken& token) :
			CancellationHandlerHolderBase(token), _handler(mutex, cond)
		{ Register(_handler); }

		~CancellationHolder()
		{
			if (TryUnregister(_handler))
				return;

			MutexUnlock ul(_handler.GetMutex());
			Unregister(_handler);
		}
	};


	class PosixConditionVariableAttr : public Singleton<PosixConditionVariableAttr>
	{
		STINGRAYKIT_SINGLETON(PosixConditionVariableAttr);

	private:
		pthread_condattr_t		_condAttr;

	private:
		PosixConditionVariableAttr()
		{
			int ret = 0;
			STINGRAYKIT_CHECK((ret = pthread_condattr_init(&_condAttr)) == 0, SystemException("pthread_condattr_init", ret));
			STINGRAYKIT_CHECK((ret = pthread_condattr_setclock(&_condAttr, CLOCK_MONOTONIC)) == 0, SystemException("pthread_condattr_setclock", ret));
		}

	public:
		~PosixConditionVariableAttr() { pthread_condattr_destroy(&_condAttr); }

		const pthread_condattr_t& Get() const { return _condAttr; }
	};


	PosixConditionVariable::PosixConditionVariable()
	{
		int ret = 0;
		STINGRAYKIT_CHECK((ret = pthread_cond_init(&_cond, &PosixConditionVariableAttr::ConstInstance().Get())) == 0, SystemException("pthread_cond_init", ret));
	}


	PosixConditionVariable::~PosixConditionVariable()
	{
		pthread_cond_destroy(&_cond);
	}


	void PosixConditionVariable::Wait(PosixMutex& mutex, const ICancellationToken& token)
	{
		CancellationHolder holder(mutex, *this, token);
		if (holder.IsCancelled())
			return;
		Wait(mutex);
	}


	bool PosixConditionVariable::TimedWait(PosixMutex& mutex, TimeDuration interval, const ICancellationToken& token)
	{
		CancellationHolder holder(mutex, *this, token);
		if (holder.IsCancelled())
			return true;
		return TimedWait(mutex, interval);
	}


	void PosixConditionVariable::Wait(const PosixMutex& mutex) const
	{
		int ret = pthread_cond_wait(&_cond, &mutex._rawMutex);
		if (ret != 0)
			STINGRAYKIT_THROW(SystemException("pthread_cond_wait", ret));
	}


	bool PosixConditionVariable::TimedWait(const PosixMutex& mutex, TimeDuration interval) const
	{
		timespec t = { };
		posix::timespec_now(CLOCK_MONOTONIC, &t);
		posix::timespec_add(&t, interval);
		int ret = pthread_cond_timedwait(&_cond, &mutex._rawMutex, &t);
		if (ret != 0 && ret != ETIMEDOUT)
			STINGRAYKIT_THROW(SystemException("pthread_cond_timedwait", ret));

		return ret != ETIMEDOUT;
	}


	void PosixConditionVariable::Broadcast()
	{
		int ret = pthread_cond_broadcast(&_cond);
		if (ret != 0)
			STINGRAYKIT_THROW(SystemException("pthread_cond_broadcast", ret));
	}

}
