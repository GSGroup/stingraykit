#ifndef STINGRAY_TOOLKIT_ATOMIC_H
#define STINGRAY_TOOLKIT_ATOMIC_H

#include <cassert>

#if HAVE_ATOMIC_H
#	include <atomic.h>
#endif

#if HAVE_SYNC_EAA
#	include <bits/atomicity.h>
#elif HAVE_SYNC_EAA_EXT
#	include <ext/atomicity.h>
#endif

#if STINGRAY_USE_HELGRIND_ANNOTATIONS
#	include <valgrind/helgrind.h>
#endif

namespace stingray
{

	/**
	 * @addtogroup toolkit_threads
	 * @{
	 */

#if HAVE_SYNC_EAA || HAVE_SYNC_EAA_EXT
	typedef _Atomic_word	atomic_int_type;
#else
	typedef int				atomic_int_type;
#endif

#if STINGRAY_USE_HELGRIND_ANNOTATIONS
#	define STINGRAY_ANNOTATE_HAPPENS_BEFORE(Marker_) \
	do \
	{ \
		ANNOTATE_HAPPENS_BEFORE(Marker_); \
	} while (0)
#	define STINGRAY_ANNOTATE_HAPPENS_AFTER(Marker_) \
	do \
	{ \
		ANNOTATE_HAPPENS_AFTER(Marker_); \
	} while (0)
#	define STINGRAY_ANNOTATE_RELEASE(Marker_) \
	do \
	{ \
		ANNOTATE_HAPPENS_BEFORE_FORGET_ALL(Marker_); \
	} while (0)
#else
#	define STINGRAY_ANNOTATE_HAPPENS_BEFORE(Marker_) \
	do { } while (0)
#	define STINGRAY_ANNOTATE_HAPPENS_AFTER(Marker_) \
	do { } while (0)
#	define STINGRAY_ANNOTATE_RELEASE(Marker_) \
	do { } while (0)
#endif


	enum memory_order
	{
		memory_order_relaxed,	// relaxed
		memory_order_consume,	// consume
		memory_order_acquire,	// acquire
		memory_order_release,	// release
		memory_order_acq_rel,	// acquire/release
		memory_order_seq_cst	// sequentially consistent
	};


	struct Atomic
	{
#if HAVE_SYNC_AAF
		template < typename T >
		static inline T Inc(T& ptr, memory_order order = memory_order_seq_cst)
		{ return __sync_add_and_fetch(&ptr, 1); }

		template < typename T >
		static inline T Dec(T& ptr, memory_order order = memory_order_seq_cst)
		{ return __sync_sub_and_fetch(&ptr, 1); }

		template < typename T1, typename T2 >
		static inline T1 Add(T1& ptr, T2 val, memory_order order = memory_order_seq_cst)
		{ return __sync_add_and_fetch(&ptr, val); }

		template < typename T1, typename T2 >
		static inline T1 Sub(T1& ptr, T2 val, memory_order order = memory_order_seq_cst)
		{ return __sync_sub_and_fetch(&ptr, val); }

		template < typename T >
		static inline T Load(T& ptr, memory_order order = memory_order_seq_cst)
		{ return __sync_add_and_fetch(&ptr, 0); }

		template < typename T1, typename T2 >
		static inline void Store(T1& ptr, T2 val, memory_order order = memory_order_seq_cst)
		{
			T1 oldval1 = 0, oldval2 = 0;
			while ((oldval1 = __sync_val_compare_and_swap(&ptr, oldval1, val)) != oldval2)
				oldval2 = oldval1;
		}
#elif HAVE_SYNC_EAA || HAVE_SYNC_EAA_EXT
		template < typename T >
		static inline T Inc(T& ptr, memory_order order = memory_order_seq_cst)
		{ return __gnu_cxx::__exchange_and_add(&ptr,  1) + 1; }

		template < typename T >
		static inline T Dec(T& ptr, memory_order order = memory_order_seq_cst)
		{ return __gnu_cxx::__exchange_and_add(&ptr, -1) - 1; }

		template < typename T1, typename T2 >
		static inline T1 Add(T1& ptr, T2 val, memory_order order = memory_order_seq_cst)
		{ return __gnu_cxx::__exchange_and_add(&ptr,  val) + val; }

		template < typename T1, typename T2 >
		static inline T1 Sub(T1& ptr, T2 val, memory_order order = memory_order_seq_cst)
		{ return __gnu_cxx::__exchange_and_add(&ptr, -val) - val; }

		template < typename T >
		static inline T Load(T& ptr, memory_order order = memory_order_seq_cst)
		{ return __gnu_cxx::__exchange_and_add(&ptr, 0); }

		template < typename T >
		static inline void Store(T& ptr, T val, memory_order order = memory_order_seq_cst)
		{ ptr = val; __gnu_cxx::__exchange_and_add(&ptr, 0); }
#elif HAVE_ATOMIC_H
		template < typename T >
		static inline T Inc(T& ptr, memory_order order = memory_order_seq_cst)
		{ return atomic_increment_val(&ptr); }

		template < typename T >
		static inline T Dec(T& ptr, memory_order order = memory_order_seq_cst)
		{ return atomic_decrement_val(&ptr); }

		template < typename T1, typename T2 >
		static inline T1 Add(T1& ptr, T2 val, memory_order order = memory_order_seq_cst)
		{ return atomic_add(&ptr,  val); }

		template < typename T1, typename T2 >
		static inline T1 Sub(T1& ptr, T2 val, memory_order order = memory_order_seq_cst)
		{ return atomic_add(&ptr, -val); }

		template < typename T >
		static inline T Load(T& ptr, memory_order order = memory_order_seq_cst)
		{ return atomic_exchange_and_add(&ptr, 0); }

		template < typename T >
		static inline void Store(T& ptr, T val, memory_order order = memory_order_seq_cst)
		{ atomic_exchange_acq(&ptr, val); }
#else
#	error "no atomic increment/decrement"
#endif

#if HAVE_SYNC_AAF
		/// @returns: value before exchange
		template < typename T >
		static inline T CompareAndExchange(T& ptr, T oldVal, T newVal) { return __sync_val_compare_and_swap(&ptr, oldVal, newVal); }
#elif HAVE_ATOMIC_H
		/// @returns: value before exchange
		template < typename T >
		static inline T CompareAndExchange(T& ptr, T oldVal, T newVal) { return atomic_compare_and_exchange_val_acq(&ptr, newVal, oldVal); }
#else
#	error "No CompareAndExchange"
#endif

#if HAVE_SYNC_AAF
		static inline bool TryLock(atomic_int_type& atomic)
		{ return __sync_lock_test_and_set(&atomic, 1) == 0; }

		static inline void Unlock(atomic_int_type& atomic)
		{ __sync_lock_release(&atomic); }
#else
		static inline bool TryLock(atomic_int_type& atomic)
		{ return (CompareAndExchange(atomic, 0, 1) == 0); }

		static inline void Unlock(atomic_int_type& atomic)
		{
			bool success = CompareAndExchange(atomic, 1, 0) == 1;
			assert(success);
		}
#endif
	};


	struct Spinlock
	{
	private:
		atomic_int_type &_lock;

	public:
		Spinlock(atomic_int_type& lock) : _lock(lock)
		{ while(!Atomic::TryLock(_lock)); }
		~Spinlock()
		{ Atomic::Unlock(_lock); }
	};

	/** @} */

}



#endif
