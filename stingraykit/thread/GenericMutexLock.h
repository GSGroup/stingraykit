#ifndef STINGRAYKIT_THREAD_GENERICMUTEXLOCK_H
#define STINGRAYKIT_THREAD_GENERICMUTEXLOCK_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/exception.h>
#include <stingraykit/fatal.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_threads
	 * @{
	 */

	template<typename T>
	class GenericMutexUnlock;

	template<typename T>
	class GenericMutexLock
	{
		STINGRAYKIT_NONCOPYABLE(GenericMutexLock);
		template <typename U> friend class GenericMutexUnlock;

	private:
		const T&		_mutex;

	public:
		inline GenericMutexLock(const T& mutex)
			: _mutex(mutex)
		{ _mutex.Lock(); }

		inline ~GenericMutexLock()
		{
			try
			{ _mutex.Unlock(); }
			catch(const std::exception& ex)
			{ STINGRAYKIT_FATAL("Couldn't unlock mutex in ~MutexLock()\n" + diagnostic_information(ex)); }
		}
	};

	template<typename T>
	class GenericMutexUnlock
	{
		STINGRAYKIT_NONCOPYABLE(GenericMutexUnlock);

	private:
		const T&		_mutex;

	public:
		inline GenericMutexUnlock(GenericMutexLock<T>& mutexLock)
			: _mutex(mutexLock._mutex)
		{ _mutex.Unlock(); }

		inline GenericMutexUnlock(const T& mutex)
			: _mutex(mutex)
		{ _mutex.Unlock(); }

		inline ~GenericMutexUnlock()
		{
			try
			{ _mutex.Lock(); }
			catch(const std::exception& ex)
			{ STINGRAYKIT_FATAL("Couldn't lock mutex in ~MutexUnlock()"); }
		}
	};

	/** @} */

}

#endif
