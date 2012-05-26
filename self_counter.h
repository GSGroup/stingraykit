#ifndef STINGRAY_SELF_COUNTER_H__
#define STINGRAY_SELF_COUNTER_H__

#include <stingray/toolkit/Atomic.h>
#include <stingray/toolkit/safe_bool.h>

#include <stingray/toolkit/exception.h>

#define TOOLKIT_DECLARE_SELF_COUNT_PTR(type) typedef stingray::self_count_ptr< type > type##SelfCountPtr;

namespace stingray
{

	template<typename T>
	class self_counter
	{
		mutable int _value;

	protected:
		~self_counter() {}

	public:
		self_counter(): _value(1) {}

		inline void add_ref() const		{ Atomic::Inc(_value); }
		inline void release_ref() const	{ if (Atomic::Dec(_value) == 0) delete static_cast<const T*>(this); }
		inline int value() const		{ return _value; }
	};


	template < typename T >
	class self_count_ptr : public safe_bool<self_count_ptr<T> >
	{
	private:
		T*				_rawPtr;

	public:
		typedef T ValueType;

		FORCE_INLINE self_count_ptr(T* rawPtr = 0)
			: _rawPtr(rawPtr) { }

		self_count_ptr(const NullPtrType&)
			: _rawPtr()
		{ }

		FORCE_INLINE self_count_ptr(const self_count_ptr<T>& other)
			: _rawPtr(other._rawPtr)
		{ if (_rawPtr) _rawPtr->add_ref(); }

		FORCE_INLINE ~self_count_ptr()
		{
			if (_rawPtr)
				_rawPtr->release_ref();
		}


		FORCE_INLINE self_count_ptr<T>& operator = (const self_count_ptr<T>& other)
		{
			self_count_ptr<T> tmp(other);
			swap(tmp);
			return *this;
		}

		FORCE_INLINE bool operator == (T* ptr) const							{ return _rawPtr == ptr; }
		FORCE_INLINE bool operator != (T* ptr) const							{ return !(*this == ptr); }
		FORCE_INLINE bool operator == (const self_count_ptr<T>& other) const	{ return other == _rawPtr; }
		FORCE_INLINE bool operator != (const self_count_ptr<T>& other) const	{ return !(*this == other); }
		FORCE_INLINE bool boolean_test() const { return _rawPtr != 0; }

		FORCE_INLINE void reset(T* ptr = 0)
		{
			self_count_ptr<T> tmp(ptr);
			swap(tmp);
		}

		FORCE_INLINE void swap(self_count_ptr<T>& other) { std::swap(_rawPtr, other._rawPtr); }

		FORCE_INLINE T* get() const				{ return _rawPtr; }
		FORCE_INLINE T* operator -> () const	{ check_ptr(); return _rawPtr; }
		FORCE_INLINE T& operator * () const		{ check_ptr(); return *_rawPtr; }
		FORCE_INLINE bool unique() const		{ return _rawPtr? _rawPtr->value() == 1: true; }

	private:

		FORCE_INLINE void check_ptr() const
		{
			if (!_rawPtr)
				TOOLKIT_THROW(NullPointerException());
		}
	};
}

#endif
