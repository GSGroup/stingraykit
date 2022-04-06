#ifndef STINGRAYKIT_THREAD_GENERICSEMAPHORELOCK_H
#define STINGRAYKIT_THREAD_GENERICSEMAPHORELOCK_H

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

	template < typename T >
	class GenericSemaphoreLock
	{
		STINGRAYKIT_NONCOPYABLE(GenericSemaphoreLock);

	private:
		T&		_semaphore;

	public:
		inline GenericSemaphoreLock(const T& semaphore)
			: _semaphore(const_cast<T &>(semaphore))
		{ _semaphore.Wait(); }

		inline ~GenericSemaphoreLock()
		{
			try
			{ _semaphore.Signal(); }
			catch (const std::exception& ex)
			{ STINGRAYKIT_FATAL("Couldn't unlock semaphore in ~SemaphoreLock()\n" + diagnostic_information(ex)); }
		}
	};

	/** @} */

}

#endif
