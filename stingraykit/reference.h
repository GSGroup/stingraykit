#ifndef STINGRAYKIT_REFERENCE_H
#define STINGRAYKIT_REFERENCE_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/toolkit.h>



namespace stingray
{


	template < typename T >
	class reference
	{
	private:
		T*		_ptr;

	public:
		explicit inline reference(T& obj)
			: _ptr(&obj)
		{ }

		inline operator T&() const
		{ return *_ptr; }
	};


	template < typename T>
	reference<T> ref(T& obj)
	{ return reference<T>(obj); }

	template < typename T>
	reference<const T> const_ref(const T& obj)
	{ return reference<const T>(obj); }

	template < typename T >
	struct ToPointerType<reference<T> >
	{ typedef T* ValueT; };

	template < typename T >
	inline T* to_pointer(const reference<T>& r) { return &(T&)r; }

}



#endif
