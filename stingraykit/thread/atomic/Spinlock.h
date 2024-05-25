#ifndef STINGRAYKIT_THREAD_ATOMIC_SPINLOCK_H
#define STINGRAYKIT_THREAD_ATOMIC_SPINLOCK_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/core/NonCopyable.h>
#include <stingraykit/thread/atomic/AtomicFlag.h>

namespace stingray
{

	class Spinlock : private NonCopyable
	{
	private:
		AtomicFlag::Type &_lock;

	public:
		Spinlock(AtomicFlag::Type& lock) : _lock(lock)
		{ while(!AtomicFlag::TestAndSet(_lock)); }

		~Spinlock()
		{ AtomicFlag::Clear(_lock); }
	};

}

#endif
