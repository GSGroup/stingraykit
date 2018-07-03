#ifndef STINGRAYKIT_THREAD_ATOMIC_H
#define STINGRAYKIT_THREAD_ATOMIC_H

// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/core/NonCopyable.h>
#include <stingraykit/thread/Thread.h>
#include <stingraykit/thread/atomic/AtomicInt.h>

namespace stingray
{

	namespace Detail
	{
		struct AtomicImplType
		{
			enum Enum
			{
				Bool,
				Integral,
				EnumClass,
				Fallback
			};
		};


		template <typename T>
		struct AtomicIntPredicate : integral_constant<bool, (sizeof(T) <= sizeof(void*))> { };

		typedef TypeListCopyIf<IntTypes, AtomicIntPredicate>::ValueT AtomicIntTypes;

		template < typename T >
		struct IsAtomicIntType : TypeListContains<AtomicIntTypes, T> { };


		template<typename T, AtomicImplType::Enum ImplType =
			IsSame<T, bool>::Value ? AtomicImplType::Bool :
			IsAtomicIntType<T>::Value ? AtomicImplType::Integral :
			IsEnumClass<T>::Value ? AtomicImplType::EnumClass : AtomicImplType::Fallback>
		class AtomicImpl;


		template < >
		class AtomicImpl<bool, AtomicImplType::Bool> : private NonCopyable
		{
		private:
			mutable AtomicU32::Type _value;

		public:
			AtomicImpl() : _value()						{ }
			AtomicImpl(bool value) : _value(value)		{ }

			bool load(MemoryOrder order) const			{ return AtomicU32::Load(_value, order); }
			void store(bool value, MemoryOrder order)	{ AtomicU32::Store(_value, value, order); }
		};


		template<typename T>
		class AtomicImpl<T, AtomicImplType::Integral> : private NonCopyable
		{
		private:
			typedef BasicAtomicInt<T> AtomicType;

		private:
			mutable typename AtomicType::Type _value;

		public:
			AtomicImpl() : _value()					{ }
			AtomicImpl(T t) : _value(t)				{ }

			T fetch_add(T arg, MemoryOrder order)	{ return AtomicType::Add(_value, arg, order) - arg; }
			T fetch_sub(T arg, MemoryOrder order)	{ return AtomicType::Sub(_value, arg, order) + arg; }

			T load(MemoryOrder order) const			{ return AtomicType::Load(_value, order); }
			void store(T value, MemoryOrder order)	{ AtomicType::Store(_value, value, order); }
		};


		template<typename T>
		class AtomicImpl<T, AtomicImplType::EnumClass> : private NonCopyable
		{
		private:
			mutable AtomicS32::Type _value;

		public:
			AtomicImpl() : _value()					{ }
			AtomicImpl(T t) : _value((s32)t)		{ }

			T load(MemoryOrder order) const			{ return (typename T::Enum)AtomicS32::Load(_value, order); }
			void store(T value, MemoryOrder order)	{ AtomicS32::Store(_value, (s32)value.val(), order); }
		};


		template<typename T>
		class AtomicImpl<T, AtomicImplType::Fallback> : private NonCopyable
		{
		private:
			Mutex	_mutex;
			T		_value;

		public:
			AtomicImpl() : _value()					{ }
			AtomicImpl(T t) : _value(t)				{ }

			T load(MemoryOrder order) const			{ MutexLock l(_mutex); return _value; }
			void store(T value, MemoryOrder order)	{ MutexLock l(_mutex); _value = value; }
		};
	}


	template<typename T>
	class atomic : private NonCopyable
	{
	private:
		Detail::AtomicImpl<T>	_impl;

	public:
		atomic()													{ }
		atomic(T t) : _impl(t)										{ }

		T load(MemoryOrder order = MemoryOrderSeqCst) const			{ return _impl.load(order); }
		void store(T value, MemoryOrder order = MemoryOrderSeqCst)	{ _impl.store(value, order); }

		T operator= (T value)										{ store(value); return value; }
		operator T () const											{ return load(); }

		T fetch_add(T arg, MemoryOrder order = MemoryOrderSeqCst)	{ return _impl.fetch_add(arg, order); }
		T fetch_sub(T arg, MemoryOrder order = MemoryOrderSeqCst)	{ return _impl.fetch_sub(arg, order); }

		T operator++()												{ return fetch_add(1) + 1; }
		T operator++(int)											{ return fetch_add(1); }

		T operator--()												{ return fetch_sub(1) - 1; }
		T operator--(int)											{ return fetch_sub(1); }
	};

}


#endif

