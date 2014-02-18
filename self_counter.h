#ifndef STINGRAY_TOOLKIT_SELF_COUNTER_H
#define STINGRAY_TOOLKIT_SELF_COUNTER_H

#include <stingray/toolkit/Atomic.h>
#include <stingray/toolkit/safe_bool.h>
#include <stingray/toolkit/exception.h>

#define TOOLKIT_DECLARE_SELF_COUNT_PTR(type) typedef stingray::self_count_ptr< type > type##SelfCountPtr;

namespace stingray
{
	namespace Detail
	{
		struct SelfCounterHelper
		{
			static void CheckAddRef(int count);
			static void CheckReleaseRef(int count);
		};
	}

	template<typename T>
	class self_counter
	{
		atomic_int_type _value;
	private:
		TOOLKIT_NONCOPYABLE(self_counter);

	protected:
		~self_counter() {}

	public:
		self_counter() : _value(1)
		{}

		inline void add_ref()
		{
			atomic_int_type count = Atomic::Inc(_value);
			TOOLKIT_DEBUG_ONLY(Detail::SelfCounterHelper::CheckAddRef(count));
		}
		inline void release_ref()
		{
			atomic_int_type count = Atomic::Dec(_value);
			if (count == 0)
			{
				STINGRAY_ANNOTATE_HAPPENS_AFTER(&_value);
				STINGRAY_ANNOTATE_RELEASE(&_value);
				delete static_cast<const T*>(this);
			}
			else
				STINGRAY_ANNOTATE_HAPPENS_BEFORE(&_value);
			TOOLKIT_DEBUG_ONLY(Detail::SelfCounterHelper::CheckReleaseRef(count));
		}
		inline int value() const		{ return _value; }
	};

	struct static_cast_tag
	{};

	template < typename T >
	class self_count_ptr : public safe_bool<self_count_ptr<T> >
	{
	private:
		template<typename U> friend class self_count_ptr;
		T*				_rawPtr;

	public:
		typedef T ValueType;

		explicit inline self_count_ptr(T* rawPtr = 0)
			: _rawPtr(rawPtr)
		{}

		self_count_ptr(const NullPtrType&)
			: _rawPtr()
		{}

		inline self_count_ptr(const self_count_ptr<T>& other)
			: _rawPtr(other._rawPtr)
		{ if (_rawPtr) _rawPtr->add_ref(); }

		template<typename U>
		inline self_count_ptr(const self_count_ptr<U>& other, typename EnableIf<Inherits<U, T>::Value, Dummy>::ValueT* = 0)
			: _rawPtr(other._rawPtr)
		{ if (_rawPtr) _rawPtr->add_ref(); }

		template<typename U>
		inline self_count_ptr(const self_count_ptr<U>& other, static_cast_tag)
			: _rawPtr(static_cast<T*>(other._rawPtr))
		{ if (_rawPtr) _rawPtr->add_ref(); }

		inline ~self_count_ptr()
		{
			if (_rawPtr)
				_rawPtr->release_ref();
		}

		inline self_count_ptr<T>& operator = (const self_count_ptr<T>& other)
		{
			self_count_ptr<T> tmp(other);
			swap(tmp);
			return *this;
		}

		inline bool operator == (T* ptr) const							{ return _rawPtr == ptr; }
		inline bool operator != (T* ptr) const							{ return !(*this == ptr); }
		inline bool operator == (const self_count_ptr<T>& other) const	{ return other == _rawPtr; }
		inline bool operator != (const self_count_ptr<T>& other) const	{ return !(*this == other); }
		inline bool boolean_test() const									{ return _rawPtr != 0; }

		inline void reset(T* ptr = 0)
		{
			self_count_ptr<T> tmp(ptr);
			swap(tmp);
		}

		inline void swap(self_count_ptr<T>& other) { std::swap(_rawPtr, other._rawPtr); }

		inline T* get() const				{ return _rawPtr; }
		inline T* operator -> () const	{ check_ptr(); return _rawPtr; }
		inline T& operator * () const		{ check_ptr(); return *_rawPtr; }
		inline bool unique() const		{ return _rawPtr? _rawPtr->value() == 1: true; }

	private:
		inline void check_ptr() const
		{
			if (!_rawPtr)
				TOOLKIT_THROW(NullPointerException());
		}
	};
}

#endif
