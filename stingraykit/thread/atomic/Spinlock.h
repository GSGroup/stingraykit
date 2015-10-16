#ifndef STINGRAYKIT_THREAD_ATOMIC_SPINLOCK_H
#define STINGRAYKIT_THREAD_ATOMIC_SPINLOCK_H

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
