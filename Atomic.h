#ifndef STINGRAY_TOOLKIT_ATOMIC_H
#define STINGRAY_TOOLKIT_ATOMIC_H

#if HAVE_ATOMIC_H
#	include <atomic.h>
#endif

#if HAVE_SYNC_EAA
#	include <bits/atomicity.h>
#endif

#include <stingray/toolkit/toolkit.h>

/*! \cond GS_INTERNAL */

namespace stingray
{

	struct Atomic
	{
#if HAVE_SYNC_AAF
		template < typename T >
		static FORCE_INLINE T Inc(T& ptr) { return __sync_add_and_fetch(&ptr, 1); }

		template < typename T >
		static FORCE_INLINE T Dec(T& ptr) { return __sync_sub_and_fetch(&ptr, 1); }
#elif HAVE_SYNC_EAA || HAVE_SYNC_EAA_EXT
		template < typename T >
		static FORCE_INLINE T Inc(T& ptr) { return __gnu_cxx::__exchange_and_add(&ptr,  1) + 1; }

		template < typename T >
		static FORCE_INLINE T Dec(T& ptr) { return __gnu_cxx::__exchange_and_add(&ptr, -1) - 1; }
#elif HAVE_ATOMIC_H
		template < typename T >
		static FORCE_INLINE T Inc(T& ptr) { return atomic_increment_val(&ptr); }

		template < typename T >
		static FORCE_INLINE T Dec(T& ptr) { return atomic_decrement_val(&ptr); }
#else
#	error "no atomic increment/decrement"
#endif

#if HAVE_SYNC_AAF
		/// @returns: value before exchange
		template < typename T >
		static FORCE_INLINE T CompareAndExchange(T& ptr, T oldVal, T newVal) { return __sync_val_compare_and_swap(&ptr, oldVal, newVal); }
#elif HAVE_ATOMIC_H
		/// @returns: value before exchange
		template < typename T >
		static FORCE_INLINE T CompareAndExchange(T& ptr, T oldVal, T newVal) { return atomic_compare_and_exchange_val_acq(&ptr, newVal, oldVal); }
#else
#	error "No CompareAndExchange"
#endif
	};

#ifdef HAVE_SYNC_EAA
	typedef _Atomic_word	atomic_int_type;
#else
	typedef int				atomic_int_type;
#endif


}

/*! \endcond */


#endif
