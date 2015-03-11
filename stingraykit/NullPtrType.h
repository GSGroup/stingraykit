#ifndef STINGRAYKIT_NULLPTRTYPE_H
#define STINGRAYKIT_NULLPTRTYPE_H

namespace stingray
{

	struct NullPtrType
	{
		template < typename T >
		operator T* () const
		{ return 0; }
	};

	extern NullPtrType null;

}

#endif
