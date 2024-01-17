#ifndef STINGRAYKIT_THREAD_ATOMIC_ATOMICINT_H
#define STINGRAYKIT_THREAD_ATOMIC_ATOMICINT_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/metaprogramming/TypeTraits.h>

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

	namespace Detail
	{
		struct FallbackMemoryOrderImpl
		{
			enum Enum
			{
				Relaxed,	// relaxed
				Consume,	// consume
				Acquire,	// acquire
				Release,	// release
				AcqRel,		// acquire/release
				SeqCst		// sequentially consistent
			};
		};


#if HAVE_SYNC_AAF

		typedef FallbackMemoryOrderImpl MemoryOrderImpl;

		template <typename IntType>
		struct AtomicIntImpl
		{
			typedef IntType Type;

			static inline IntType Inc(Type& atomic, MemoryOrderImpl::Enum order)              { return __sync_add_and_fetch(&atomic, 1); }
			static inline IntType Dec(Type& atomic, MemoryOrderImpl::Enum order)              { return __sync_sub_and_fetch(&atomic, 1); }
			static inline IntType Add(Type& atomic, IntType val, MemoryOrderImpl::Enum order) { return __sync_add_and_fetch(&atomic, val); }
			static inline IntType Sub(Type& atomic, IntType val, MemoryOrderImpl::Enum order) { return __sync_sub_and_fetch(&atomic, val); }
			static inline IntType Load(Type& atomic, MemoryOrderImpl::Enum order)             { return __sync_add_and_fetch(&atomic, 0); }

			static inline void Store(Type& atomic, IntType val, MemoryOrderImpl::Enum order)
			{
				IntType oldval1 = 0, oldval2 = 0;
				while ((oldval1 = __sync_val_compare_and_swap(&atomic, oldval1, val)) != oldval2)
					oldval2 = oldval1;
			}

			static inline IntType CompareAndExchange(Type& atomic, IntType oldVal, IntType newVal)
			{ return __sync_val_compare_and_swap(&atomic, oldVal, newVal); }
		};

#elif HAVE_ATOMIC_BUILTINS

		struct MemoryOrderImpl
		{
			enum Enum
			{
				Relaxed = __ATOMIC_RELAXED,	// relaxed
				Consume = __ATOMIC_CONSUME,	// consume
				Acquire = __ATOMIC_ACQUIRE,	// acquire
				Release = __ATOMIC_RELEASE,	// release
				AcqRel  = __ATOMIC_ACQ_REL,	// acquire/release
				SeqCst  = __ATOMIC_SEQ_CST	// sequentially consistent
			};
		};

		template <typename IntType>
		struct AtomicIntImpl
		{
			typedef IntType Type;

			static inline IntType Inc(Type& atomic, MemoryOrderImpl::Enum order)              { return __atomic_add_fetch(&atomic, 1, order); }
			static inline IntType Dec(Type& atomic, MemoryOrderImpl::Enum order)              { return __atomic_sub_fetch(&atomic, 1, order); }
			static inline IntType Add(Type& atomic, IntType val, MemoryOrderImpl::Enum order) { return __atomic_add_fetch(&atomic, val, order); }
			static inline IntType Sub(Type& atomic, IntType val, MemoryOrderImpl::Enum order) { return __atomic_sub_fetch(&atomic, val, order); }
			static inline IntType Load(Type& atomic, MemoryOrderImpl::Enum order)             { return __atomic_load_n(&atomic, order); }
			static inline void Store(Type& atomic, IntType val, MemoryOrderImpl::Enum order)  { __atomic_store_n(&atomic, val, order); }

#error "No CompareAndExchange implemented"
		};

#elif HAVE_SYNC_EAA || HAVE_SYNC_EAA_EXT

		typedef FallbackMemoryOrderImpl MemoryOrderImpl;

		template <typename IntType>
		struct AtomicIntImpl
		{
			typedef _Atomic_word Type;

			static inline IntType Inc(Type& atomic, MemoryOrderImpl::Enum order)              { CheckType(); return __gnu_cxx::__exchange_and_add(&ptr,  1) + 1; }
			static inline IntType Dec(Type& atomic, MemoryOrderImpl::Enum order)              { CheckType(); return __gnu_cxx::__exchange_and_add(&ptr, -1) - 1; }
			static inline IntType Add(Type& atomic, IntType val, MemoryOrderImpl::Enum order) { CheckType(); return __gnu_cxx::__exchange_and_add(&ptr,  val) + val; }
			static inline IntType Sub(Type& atomic, IntType val, MemoryOrderImpl::Enum order) { CheckType(); return __gnu_cxx::__exchange_and_add(&ptr, -val) - val; }
			static inline IntType Load(Type& atomic, MemoryOrderImpl::Enum order)             { CheckType(); return __gnu_cxx::__exchange_and_add(&ptr, 0); }
			static inline void Store(Type& atomic, IntType val, MemoryOrderImpl::Enum order)  { CheckType(); ptr = val; __gnu_cxx::__exchange_and_add(&ptr, 0); }

#error "No CompareAndExchange implemented"

		private:
			void CheckType()
			{
				static_assert(sizeof(_Atomic_word) == sizeof(IntType), "Size mismatch");
				static_assert(IntTraits<IntType>::IsSigned, "No unsigned atomic types implemented");
			}
		};

#elif HAVE_ATOMIC_H

		typedef FallbackMemoryOrderImpl MemoryOrderImpl;

		template <typename IntType>
		struct AtomicIntImpl
		{
			typedef IntType Type;

			static inline IntType Inc(Type& atomic, MemoryOrderImpl::Enum order)                   { return atomic_increment_val(&ptr); }
			static inline IntType Dec(Type& atomic, MemoryOrderImpl::Enum order)                   { return atomic_decrement_val(&ptr); }
			static inline IntType Add(Type& atomic, IntType val, MemoryOrderImpl::Enum order)      { return atomic_add(&ptr,  val); }
			static inline IntType Sub(Type& atomic, IntType val, MemoryOrderImpl::Enum order)      { return atomic_add(&ptr, -val); }
			static inline IntType Load(Type& atomic, MemoryOrderImpl::Enum order)                  { return atomic_exchange_and_add(&ptr, 0); }
			static inline void Store(Type& atomic, IntType val, MemoryOrderImpl::Enum order)       { atomic_exchange_acq(&ptr, val); }

			static inline IntType CompareAndExchange(Type& atomic, IntType oldVal, IntType newVal) { return atomic_compare_and_exchange_val_acq(&atomic, newVal, oldVal); }
		};

#else
#	error "No atomics implemented!"
#endif
	}


#if STINGRAY_USE_HELGRIND_ANNOTATIONS
#	define STINGRAYKIT_ANNOTATE_HAPPENS_BEFORE(Marker_) \
	do \
	{ \
		ANNOTATE_HAPPENS_BEFORE(Marker_); \
	} while (0)
#	define STINGRAYKIT_ANNOTATE_HAPPENS_AFTER(Marker_) \
	do \
	{ \
		ANNOTATE_HAPPENS_AFTER(Marker_); \
	} while (0)
#	define STINGRAYKIT_ANNOTATE_RELEASE(Marker_) \
	do \
	{ \
		ANNOTATE_HAPPENS_BEFORE_FORGET_ALL(Marker_); \
	} while (0)
#else
#	define STINGRAYKIT_ANNOTATE_HAPPENS_BEFORE(Marker_) \
	do { } while (0)
#	define STINGRAYKIT_ANNOTATE_HAPPENS_AFTER(Marker_) \
	do { } while (0)
#	define STINGRAYKIT_ANNOTATE_RELEASE(Marker_) \
	do { } while (0)
#endif


	enum MemoryOrder
	{
		MemoryOrderRelaxed = Detail::MemoryOrderImpl::Relaxed,
		MemoryOrderConsume = Detail::MemoryOrderImpl::Consume,
		MemoryOrderAcquire = Detail::MemoryOrderImpl::Acquire,
		MemoryOrderRelease = Detail::MemoryOrderImpl::Release,
		MemoryOrderAcqRel  = Detail::MemoryOrderImpl::AcqRel,
		MemoryOrderSeqCst  = Detail::MemoryOrderImpl::SeqCst
	};


	template <typename IntType>
	struct BasicAtomicInt
	{
		typedef typename Detail::AtomicIntImpl<IntType>::Type Type;

		/// @brief Increment atomic integer
		/// @returns New value (i.e. value after incrementing)
		static inline IntType Inc(Type& atomic, MemoryOrder order = MemoryOrderSeqCst)
		{ return Detail::AtomicIntImpl<IntType>::Inc(atomic, (Detail::MemoryOrderImpl::Enum)order); }

		/// @brief Decrement atomic integer
		/// @returns New value (i.e. value after decrementing)
		static inline IntType Dec(Type& atomic, MemoryOrder order = MemoryOrderSeqCst)
		{ return Detail::AtomicIntImpl<IntType>::Dec(atomic, (Detail::MemoryOrderImpl::Enum)order); }

		/// @brief Add to atomic integer
		/// @returns New value (i.e. value after addition)
		static inline IntType Add(Type& atomic, IntType val, MemoryOrder order = MemoryOrderSeqCst)
		{ return Detail::AtomicIntImpl<IntType>::Add(atomic, val, (Detail::MemoryOrderImpl::Enum)order); }

		/// @brief Subtract from atomic integer
		/// @returns New value (i.e. value after subtraction)
		static inline IntType Sub(Type& atomic, IntType val, MemoryOrder order = MemoryOrderSeqCst)
		{ return Detail::AtomicIntImpl<IntType>::Sub(atomic, val, (Detail::MemoryOrderImpl::Enum)order); }

		/// @brief Atomically get integer value
		/// @returns Loaded value
		static inline IntType Load(Type& atomic, MemoryOrder order = MemoryOrderSeqCst)
		{ return Detail::AtomicIntImpl<IntType>::Load(atomic, (Detail::MemoryOrderImpl::Enum)order); }

		/// @brief Atomically set integer value
		static inline void Store(Type& atomic, IntType val, MemoryOrder order = MemoryOrderSeqCst)
		{ Detail::AtomicIntImpl<IntType>::Store(atomic, val, (Detail::MemoryOrderImpl::Enum)order); }

		/// @brief Compare atomic integer value to an oldVal. If they are equal - set atomic integer value to newVal, otherwise do nothing
		/// @returns Value before exchange. So, if returned value is equal to oldVal, it means that CompareAndExchange set atomic value to newVal
		static inline IntType CompareAndExchange(Type& atomic, IntType oldVal, IntType newVal)
		{ return Detail::AtomicIntImpl<IntType>::CompareAndExchange(atomic, oldVal, newVal); }
	};

	typedef BasicAtomicInt<u32> AtomicU32;
	typedef BasicAtomicInt<s32> AtomicS32;
	typedef BasicAtomicInt<u64> AtomicU64;
	typedef BasicAtomicInt<s64> AtomicS64;

	/** @} */

}



#endif
