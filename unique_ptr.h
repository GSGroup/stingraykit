#ifndef STINGRAY_TOOLKIT_UNIQUE_PTR_H
#define STINGRAY_TOOLKIT_UNIQUE_PTR_H

#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/safe_bool.h>
#include <stingray/toolkit/toolkit.h>

namespace stingray
{


	template<typename T>
	class unique_ptr : public safe_bool<unique_ptr<T> >
	{
		TOOLKIT_NONCOPYABLE(unique_ptr);
		T* _rawPtr;

	public:
		explicit FORCE_INLINE unique_ptr(T* rawPtr = 0) :
			_rawPtr(rawPtr)
		{}

		~unique_ptr()
		{ delete _rawPtr; }

		FORCE_INLINE bool operator == (T* ptr) const						{ return _rawPtr == ptr; }
		FORCE_INLINE bool operator != (T* ptr) const						{ return !(*this == ptr); }
		FORCE_INLINE bool operator == (const unique_ptr<T>& other) const	{ return other == _rawPtr; }
		FORCE_INLINE bool operator != (const unique_ptr<T>& other) const	{ return !(*this == other); }
		FORCE_INLINE bool boolean_test() const								{ return _rawPtr != 0; }
		FORCE_INLINE T* get() const											{ return _rawPtr; }
		FORCE_INLINE T* operator -> () const								{ check_ptr(); return _rawPtr; }
		FORCE_INLINE T& operator * () const									{ check_ptr(); return *_rawPtr; }
		FORCE_INLINE T* release()											{ T* ptr = _rawPtr; _rawPtr = 0; return ptr; }
		FORCE_INLINE void swap(unique_ptr<T>& other)						{ std::swap(_rawPtr, other._rawPtr); }

		FORCE_INLINE void reset(T* ptr = 0)
		{
			delete _rawPtr;
			_rawPtr = ptr;
		}

	private:
		FORCE_INLINE void check_ptr() const
		{ TOOLKIT_CHECK(_rawPtr, NullPointerException()); }
	};


}

#endif
