#ifndef STINGRAYKIT_UNIQUE_PTR_H
#define STINGRAYKIT_UNIQUE_PTR_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/exception.h>

namespace stingray
{

	template < typename T >
	class unique_ptr
	{
		STINGRAYKIT_NONCOPYABLE(unique_ptr);

	private:
		T* _rawPtr;

	public:
		explicit unique_ptr(T* rawPtr = 0) : _rawPtr(rawPtr)
		{ }

		~unique_ptr()
		{ CheckedDelete(_rawPtr); }

		bool operator == (T* ptr) const							{ return _rawPtr == ptr; }
		bool operator != (T* ptr) const							{ return !(*this == ptr); }
		bool operator == (const unique_ptr& other) const		{ return other == _rawPtr; }
		bool operator != (const unique_ptr& other) const		{ return !(*this == other); }

		bool is_initialized() const								{ return _rawPtr != 0; }
		explicit operator bool () const							{ return is_initialized(); }

		T* get() const											{ return _rawPtr; }
		T* operator -> () const									{ check_ptr(); return _rawPtr; }
		T& operator * () const									{ check_ptr(); return *_rawPtr; }
		T* release()											{ T* ptr = _rawPtr; _rawPtr = 0; return ptr; }
		void swap(unique_ptr& other)							{ std::swap(_rawPtr, other._rawPtr); }

		void reset(T* ptr = 0)
		{
			CheckedDelete(_rawPtr);
			_rawPtr = ptr;
		}

	private:
		void check_ptr() const
		{ STINGRAYKIT_CHECK(_rawPtr, NullPointerException()); }
	};


	template < typename T >
	class unique_ptr<T[]>
	{
		STINGRAYKIT_NONCOPYABLE(unique_ptr);

	private:
		T* _rawPtr;

	public:
		explicit unique_ptr(T* rawPtr = 0) : _rawPtr(rawPtr)
		{ }

		~unique_ptr()
		{ CheckedArrayDelete(_rawPtr); }

		bool operator == (T* ptr) const							{ return _rawPtr == ptr; }
		bool operator != (T* ptr) const							{ return !(*this == ptr); }
		bool operator == (const unique_ptr<T[]>& other) const	{ return other == _rawPtr; }
		bool operator != (const unique_ptr<T[]>& other) const	{ return !(*this == other); }

		bool is_initialized() const								{ return _rawPtr != 0; }
		explicit operator bool () const							{ return is_initialized(); }

		T* get() const											{ return _rawPtr; }
		T& operator [] (size_t i) const							{ check_ptr(); return _rawPtr[i]; }
		T* release()											{ T* ptr = _rawPtr; _rawPtr = 0; return ptr; }
		void swap(unique_ptr<T[]>& other)						{ std::swap(_rawPtr, other._rawPtr); }

		void reset(T* ptr = 0)
		{
			CheckedArrayDelete(_rawPtr);
			_rawPtr = ptr;
		}

	private:
		void check_ptr() const
		{ STINGRAYKIT_CHECK(_rawPtr, NullPointerException()); }
	};

}

#endif
