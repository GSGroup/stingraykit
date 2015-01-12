#ifndef STINGRAY_TOOLKIT_THREAD_GENERICSEMAPHORELOCK_H
#define STINGRAY_TOOLKIT_THREAD_GENERICSEMAPHORELOCK_H

namespace stingray
{

	/**
	 * @addtogroup toolkit_threads
	 * @{
	 */

    template<typename T>
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
			catch(const std::exception& ex)
			{ STINGRAYKIT_FATAL(StringBuilder() % "Couldn't unlock semaphore in ~SemaphoreLock()\n" % ex); }
		}
	};

	/** @} */

}

#endif
