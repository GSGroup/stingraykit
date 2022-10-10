// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/posix/PosixConditionVariable.h>

#include <stingraykit/thread/CancellationRegistrator.h>
#include <stingraykit/time/posix/utils.h>
#include <stingraykit/Singleton.h>

#include <errno.h>

namespace stingray
{

	class CancellationHolder : public CancellationRegistratorBase
	{
		class CancellationHandler : public ICancellationHandler
		{
		private:
			const PosixMutex&		_mutex;
			PosixConditionVariable&	_cond;

		public:
			CancellationHandler(const PosixMutex& mutex, PosixConditionVariable& cond)
				: _mutex(mutex), _cond(cond)
			{ }

			virtual ~CancellationHandler()
			{ }

			virtual void Cancel()
			{
				MutexLock l(_mutex);
				_cond.Broadcast();
			}

			const PosixMutex& GetMutex() const
			{ return _mutex; }
		};

	private:
		CancellationHandler	_handler;

	public:
		CancellationHolder(const PosixMutex& mutex, PosixConditionVariable& cond, const ICancellationToken& token)
			: CancellationRegistratorBase(token), _handler(mutex, cond)
		{ Register(_handler); }

		~CancellationHolder()
		{
			if (TryUnregister(_handler))
				return;

			MutexUnlock ul(_handler.GetMutex());
			Unregister(_handler);
		}

		bool Release()
		{ return TryUnregister(_handler); }
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


	ConditionWaitResult PosixConditionVariable::Wait(const PosixMutex& mutex, const ICancellationToken& token)
	{
		CancellationHolder holder(mutex, *this, token);
		if (holder.IsCancelled())
			return ConditionWaitResult::Cancelled;

		const optional<TimeDuration> timeout = token.GetTimeout();
		if (!timeout)
			Wait(mutex);
		else if (!TimedWait(mutex, *timeout))
			return ConditionWaitResult::TimedOut;

		if (!holder.Release())
			return ConditionWaitResult::Cancelled;

		return ConditionWaitResult::Broadcasted;
	}


	void PosixConditionVariable::Wait(const PosixMutex& mutex) const
	{
		int ret = pthread_cond_wait(&_cond, &mutex._rawMutex);
		STINGRAYKIT_CHECK(ret == 0, SystemException("pthread_cond_wait", ret));
	}


	bool PosixConditionVariable::TimedWait(const PosixMutex& mutex, TimeDuration interval) const
	{
		timespec t = { };
		posix::timespec_now(CLOCK_MONOTONIC, &t);
		posix::timespec_add(&t, interval);
		int ret = pthread_cond_timedwait(&_cond, &mutex._rawMutex, &t);
		STINGRAYKIT_CHECK(ret == 0 || ret == ETIMEDOUT, SystemException("pthread_cond_timedwait", ret));

		return ret != ETIMEDOUT;
	}


	void PosixConditionVariable::Broadcast()
	{
		int ret = pthread_cond_broadcast(&_cond);
		STINGRAYKIT_CHECK(ret == 0, SystemException("pthread_cond_broadcast", ret));
	}

}
