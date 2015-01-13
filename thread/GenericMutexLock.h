#ifndef STINGRAYKIT_THREAD_GENERICMUTEXLOCK_H
#define STINGRAYKIT_THREAD_GENERICMUTEXLOCK_H

#include <stingray/toolkit/fatal.h>

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
			{ STINGRAYKIT_FATAL(StringBuilder() % "Couldn't unlock mutex in ~MutexLock()\n" % ex); }
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
