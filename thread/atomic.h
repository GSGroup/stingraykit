#ifndef STINGRAY_TOOLKIT_THREAD_ATOMIC_H
#define STINGRAY_TOOLKIT_THREAD_ATOMIC_H

#include <stingray/toolkit/thread/Thread.h>
#include <stingray/toolkit/Atomic.h>

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

