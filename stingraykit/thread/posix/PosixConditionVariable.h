#ifndef STINGRAYKIT_THREAD_POSIX_POSIXCONDITIONVARIABLE_H
#define STINGRAYKIT_THREAD_POSIX_POSIXCONDITIONVARIABLE_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/posix/PosixThreadEngine.h>
#include <stingraykit/thread/ICancellationToken.h>
#include <stingraykit/time/Time.h>

#include <pthread.h>

namespace stingray
{

	struct ConditionWaitResult
	{
		STINGRAYKIT_ENUM_VALUES
		(
			Broadcasted,
			Cancelled,
			TimedOut
		);

		STINGRAYKIT_DECLARE_ENUM_CLASS(ConditionWaitResult);
	};


	class PosixConditionVariable
	{
		STINGRAYKIT_NONCOPYABLE(PosixConditionVariable);

	private:
		mutable pthread_cond_t	_cond;

	public:
		PosixConditionVariable();
		~PosixConditionVariable();

		ConditionWaitResult Wait(const PosixMutex& mutex, const ICancellationToken& token);
		bool TimedWait(const PosixMutex& mutex, TimeDuration interval, const ICancellationToken& token);

		void Wait(const PosixMutex& mutex) const;
		bool TimedWait(const PosixMutex& mutex, TimeDuration interval) const;

		void Broadcast();
	};

}

#endif
