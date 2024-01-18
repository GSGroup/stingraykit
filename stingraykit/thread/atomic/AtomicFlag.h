// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#ifndef STINGRAYKIT_THREAD_ATOMIC_ATOMICFLAG_H
#define STINGRAYKIT_THREAD_ATOMIC_ATOMICFLAG_H

namespace stingray
{

	namespace Detail
	{
#if HAVE_SYNC_AAF
		struct AtomicFlagImpl
		{
			typedef int Type;

			static inline bool TestAndSet(Type& flag) { return __sync_lock_test_and_set(&flag, 1) == 0; }
			static inline void Clear(Type& flag)      { __sync_lock_release(&flag); }
		};
#else
		struct AtomicFlagImpl
		{
			typedef AtomicInt::Type Type;

			static inline bool TestAndSet(Type& flag) { return (CompareAndExchange(flag, 0, 1) == 0); }
			static inline void Clear(Type& flag)      { CompareAndExchange(flag, 1, 0) == 1; }
		};
#endif
	}


	struct AtomicFlag
	{
		typedef Detail::AtomicFlagImpl::Type Type;

		static inline bool TestAndSet(Type& flag) { return Detail::AtomicFlagImpl::TestAndSet(flag); }
		static inline void Clear(Type& flag)      { Detail::AtomicFlagImpl::Clear(flag); }
	};

}

#endif
