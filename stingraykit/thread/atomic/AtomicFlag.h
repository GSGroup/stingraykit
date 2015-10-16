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
