#ifndef __GS_DVRLIB_TOOLKIT_ENABLE_SHARED_FROM_THIS_H__
#define __GS_DVRLIB_TOOLKIT_ENABLE_SHARED_FROM_THIS_H__


#include <stingray/toolkit/shared_ptr.h>


/*! \cond GS_INTERNAL */

namespace stingray
{


	// Still a bit buggy
	template < typename T >
	class enable_shared_from_this
	{
		template < typename U >
		friend class shared_ptr;

	private:
		mutable weak_ptr<T>		_thisPtr;

	protected:
		shared_ptr<T> shared_from_this() const
		{ return _thisPtr.lock(); };

	private:
		void init(const shared_ptr<T>& thisPtr) const
		{ _thisPtr = thisPtr; }
	};


};

/*! \endcond */


#endif
