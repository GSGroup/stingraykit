#ifndef __GS_DVRLIB_TOOLKIT_REFERENCE_H__
#define __GS_DVRLIB_TOOLKIT_REFERENCE_H__


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
		explicit FORCE_INLINE reference(T& obj)
			: _ptr(&obj)
		{ }

		FORCE_INLINE operator T&() const
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
	FORCE_INLINE T* to_pointer(const reference<T>& r) { return &(T&)r; }

}

/*! \endcond */


#endif
