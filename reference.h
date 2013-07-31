#ifndef STINGRAY_TOOLKIT_REFERENCE_H
#define STINGRAY_TOOLKIT_REFERENCE_H


#include <stingray/toolkit/toolkit.h>


/*! \cond GS_INTERNAL */

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

/*! \endcond */


#endif
