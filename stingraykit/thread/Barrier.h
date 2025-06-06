#ifndef STINGRAYKIT_THREAD_BARRIER_H
#define STINGRAYKIT_THREAD_BARRIER_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/ConditionVariable.h>
#include <stingraykit/thread/Thread.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_threads
	 * @{
	 */

	class Barrier
	{
	private:
		const size_t		_requiredCount;
		size_t				_currentCount;
		bool				_releasing;
		Mutex				_mutex;
		ConditionVariable	_cv;

	public:
		Barrier(size_t count) : _requiredCount(count), _currentCount(0), _releasing(false)
		{}

		~Barrier()
		{}

		bool Wait()
		{
			MutexLock l(_mutex);
			while (_releasing)
				_cv.Wait(_mutex);
			if (++_currentCount == _requiredCount)
			{
				_releasing = true;
				_cv.Broadcast();
			}
			else
				_cv.Wait(_mutex);
			if (--_currentCount == 0)
			{
				_releasing = false;
				_cv.Broadcast();
				return true;
			}
			return false;
		}
	};

	/** @} */

}


#endif

