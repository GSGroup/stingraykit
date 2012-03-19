#ifndef __GS_DVRLIB_TOOLKIT_ATOMIC_H__
#define __GS_DVRLIB_TOOLKIT_ATOMIC_H__

#if HAVE_ATOMIC_H
#	include <atomic.h>
#elif HAVE_SYNC_EAA
#	include <bits/atomicity.h>
#endif
#include <dvrlib/toolkit/toolkit.h>

/*! \cond GS_INTERNAL */

namespace dvrlib
{

	struct Atomic
	{
#if HAVE_SYNC_AAF
		template < typename T>
		static FORCE_INLINE T Inc(T& ptr) { return __sync_add_and_fetch(&ptr, 1); }

		template < typename T>
		static FORCE_INLINE T Dec(T& ptr) { return __sync_sub_and_fetch(&ptr, 1); }
#elif HAVE_SYNC_EAA || HAVE_SYNC_EAA_EXT
		template < typename T>
		static FORCE_INLINE T Inc(T& ptr) { return __gnu_cxx::__exchange_and_add(&ptr,  1) + 1; }

		template < typename T>
		static FORCE_INLINE T Dec(T& ptr) { return __gnu_cxx::__exchange_and_add(&ptr, -1) - 1; }
#elif HAVE_ATOMIC_H
		template < typename T>
		static FORCE_INLINE T Inc(T& ptr) { return atomic_increment_val(&ptr); }

		template < typename T>
		static FORCE_INLINE T Dec(T& ptr) { return atomic_decrement_val(&ptr); }
#else
#	error "no atomic operations"
#endif
	};

}

/*! \endcond */


#endif
