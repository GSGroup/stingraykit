#ifndef STINGRAYKIT_THREAD_LOCKPROFILER_H
#define STINGRAYKIT_THREAD_LOCKPROFILER_H

#include <stingray/toolkit/time/ElapsedTime.h>
#include <stingray/toolkit/diagnostics/Backtrace.h>
#include <stdio.h>

namespace stingray
{
	template<typename LockType_, int Threshold = 100>
	class MutexProfilerWrapper : public LockType_
	{
		typedef LockType_ LockType;

		mutable ElapsedTime		_elapsed;
		mutable bool			_enabled;

	public:
		inline MutexProfilerWrapper(): _enabled(true) {}

		void ShutUp()
		{
			LockType::Lock();
			_enabled = false;
			LockType::Unlock();
		}

		inline void Lock() const
		{
			LockType::Lock();
			_elapsed.Restart();
		}

		inline bool TryLock() const
		{
			bool r = LockType::TryLock();
			if (r)
			{
				_elapsed.Restart();
			}
			return r;
		}

		inline void Unlock() const
		{
			if (_enabled)
			{
				s64 elapsed = _elapsed.ElapsedMilliseconds();
				if (elapsed >= Threshold)
				{
					Backtrace bt;
					printf("mutex was locked for %ld ms at %s\n", (long)elapsed, bt.Get().c_str());
				}
			}
			LockType::Unlock();
		}
	};
}

#endif
