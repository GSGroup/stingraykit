#ifndef STINGRAY_TOOLKIT_ATOMIC_H
#define STINGRAY_TOOLKIT_ATOMIC_H

#if HAVE_ATOMIC_H
#	include <atomic.h>
#endif

#if HAVE_SYNC_EAA
#	include <bits/atomicity.h>
#endif

/*! \cond GS_INTERNAL */

namespace stingray
{

#ifdef HAVE_SYNC_EAA
	typedef _Atomic_word	atomic_int_type;
#else
	typedef int				atomic_int_type;
#endif

	struct Atomic
	{
#if HAVE_SYNC_AAF
		template < typename T >
		static inline T Inc(T& ptr) { return __sync_add_and_fetch(&ptr, 1); }

		template < typename T >
		static inline T Dec(T& ptr) { return __sync_sub_and_fetch(&ptr, 1); }

		template < typename T >
		static inline T Load(T& ptr) { return __sync_add_and_fetch(&ptr, 0); }

		template < typename T1, typename T2 >
		static inline void Store(T1& ptr, T2 val)
		{
			ptr = val;
			__sync_synchronize();
		}
#elif HAVE_SYNC_EAA || HAVE_SYNC_EAA_EXT
		template < typename T >
		static inline T Inc(T& ptr) { return __gnu_cxx::__exchange_and_add(&ptr,  1) + 1; }

		template < typename T >
		static inline T Dec(T& ptr) { return __gnu_cxx::__exchange_and_add(&ptr, -1) - 1; }

		template < typename T >
		static inline T Load(T& ptr) { return __gnu_cxx::__exchange_and_add(&ptr, 0); }
		// TODO: Implement Store
#elif HAVE_ATOMIC_H
		template < typename T >
		static inline T Inc(T& ptr) { return atomic_increment_val(&ptr); }

		template < typename T >
		static inline T Dec(T& ptr) { return atomic_decrement_val(&ptr); }

		template < typename T >
		static inline T Load(T& ptr) { return atomic_exchange_and_add(&ptr, 0); }

		template < typename T >
		static inline void Store(T& ptr, T val) { atomic_exchange_acq(&ptr, val); }
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
	};



}

/*! \endcond */


#endif
