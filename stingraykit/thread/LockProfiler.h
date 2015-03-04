#ifndef STINGRAYKIT_THREAD_LOCKPROFILER_H
#define STINGRAYKIT_THREAD_LOCKPROFILER_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/time/ElapsedTime.h>
#include <stingraykit/diagnostics/Backtrace.h>
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
