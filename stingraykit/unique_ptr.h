#ifndef STINGRAYKIT_UNIQUE_PTR_H
#define STINGRAYKIT_UNIQUE_PTR_H

#include <stingraykit/exception.h>
#include <stingraykit/safe_bool.h>
#include <stingraykit/toolkit.h>

namespace stingray
{


	template<typename T>
	class unique_ptr : public safe_bool<unique_ptr<T> >
	{
		STINGRAYKIT_NONCOPYABLE(unique_ptr);
		T* _rawPtr;

	public:
		explicit inline unique_ptr(T* rawPtr = 0) :
			_rawPtr(rawPtr)
		{}

		~unique_ptr()
		{ delete _rawPtr; }

		inline bool operator == (T* ptr) const						{ return _rawPtr == ptr; }
		inline bool operator != (T* ptr) const						{ return !(*this == ptr); }
		inline bool operator == (const unique_ptr<T>& other) const	{ return other == _rawPtr; }
		inline bool operator != (const unique_ptr<T>& other) const	{ return !(*this == other); }
		inline bool boolean_test() const							{ return _rawPtr != 0; }
		inline T* get() const										{ return _rawPtr; }
		inline T* operator -> () const								{ check_ptr(); return _rawPtr; }
		inline T& operator * () const								{ check_ptr(); return *_rawPtr; }
		inline T* release()											{ T* ptr = _rawPtr; _rawPtr = 0; return ptr; }
		inline void swap(unique_ptr<T>& other)						{ std::swap(_rawPtr, other._rawPtr); }

		inline void reset(T* ptr = 0)
		{
			delete _rawPtr;
			_rawPtr = ptr;
		}

	private:
		inline void check_ptr() const
		{ STINGRAYKIT_CHECK(_rawPtr, NullPointerException()); }
	};


}

#endif
