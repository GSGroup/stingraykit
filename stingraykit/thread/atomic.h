#ifndef STINGRAYKIT_THREAD_ATOMIC_H
#define STINGRAYKIT_THREAD_ATOMIC_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/Thread.h>
#include <stingraykit/Atomic.h>

namespace stingray
{

	namespace Detail
	{
		struct AtomicImplType
		{
			STINGRAYKIT_ENUM_VALUES(
				Integral,
				EnumClass,
				Fallback);

			STINGRAYKIT_DECLARE_ENUM_CLASS(AtomicImplType);
		};


		template<typename T, AtomicImplType::Enum ImplType =
			IsIntType<T>::Value ? AtomicImplType::Integral :
			IsEnumClass<T>::Value ? AtomicImplType::EnumClass : AtomicImplType::Fallback>
		class AtomicImpl;

		template<typename T>
		class AtomicImpl<T, AtomicImplType::Integral>
		{
			STINGRAYKIT_NONCOPYABLE(AtomicImpl);

		private:
			typedef atomic_int_type AtomicType;
			typedef typename EnableIf<sizeof(AtomicType) >= sizeof(T), AtomicType>::ValueT	Value;

		private:
			mutable Value _value;

		public:
			AtomicImpl()							{ }
			AtomicImpl(T t) : _value((Value)t)		{ }

			T fetch_add(T arg, memory_order order) { return (T)Atomic::Add(_value, (Value)arg, order) - 1; }
			T fetch_sub(T arg, memory_order order) { return (T)Atomic::Sub(_value, (Value)arg, order) + 1; }

			T load(memory_order order) const		{ return (T)Atomic::Load(_value, order); }
			void store(T value, memory_order order)	{ Atomic::Store(_value, (Value)value, order); }
		};


		template<typename T>
		class AtomicImpl<T, AtomicImplType::EnumClass>
		{
			STINGRAYKIT_NONCOPYABLE(AtomicImpl);

		private:
			typedef atomic_int_type AtomicType;
			typedef typename EnableIf<sizeof(AtomicType) >= sizeof(typename T::Enum), AtomicType>::ValueT	Value;

		private:
			mutable Value _value;

		public:
			AtomicImpl()							{ }
			AtomicImpl(T t) : _value((Value)t)		{ }

			T load(memory_order order) const		{ return (typename T::Enum)Atomic::Load(_value, order); }
			void store(T value, memory_order order)	{ Atomic::Store(_value, (Value)value.val(), order); }
		};


		template<typename T>
		class AtomicImpl<T, AtomicImplType::Fallback>
		{
			STINGRAYKIT_NONCOPYABLE(AtomicImpl);

		private:
			Mutex	_mutex;
			T		_value;

		public:
			AtomicImpl()							{ }
			AtomicImpl(T t) : _value(t)				{ }

			T load(memory_order order) const		{ MutexLock l(_mutex); return _value; }
			void store(T value, memory_order order)	{ MutexLock l(_mutex); _value = value; }
		};
	}


	template<typename T>
	class atomic
	{
		STINGRAYKIT_NONCOPYABLE(atomic);

	private:
		Detail::AtomicImpl<T>	_impl;

	public:
		atomic()														{ }
		atomic(T t) : _impl(t)											{ }

		T load(memory_order order = memory_order_seq_cst) const			{ return _impl.load(order); }
		void store(T value, memory_order order = memory_order_seq_cst)	{ _impl.store(value, order); }

		T operator= (T value)											{ store(value); return value; }
		operator T () const												{ return load(); }

		T fetch_add(T arg, memory_order order = memory_order_seq_cst)	{ return _impl.fetch_add(arg, order); }
		T fetch_sub(T arg, memory_order order = memory_order_seq_cst)	{ return _impl.fetch_sub(arg, order); }

		T operator++()													{ return fetch_add(1) + 1; }
		T operator++(int)												{ return fetch_add(1); }

		T operator--()													{ return fetch_sub(1) - 1; }
		T operator--(int)												{ return fetch_sub(1); }
	};

}


#endif

