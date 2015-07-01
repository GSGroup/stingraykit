#ifndef STINGRAYKIT_CHECKEDDELETE_H
#define STINGRAYKIT_CHECKEDDELETE_H

#include <stingraykit/metaprogramming/TypeTraits.h>

namespace stingray
{

	template <typename T>
	inline void CheckedDelete(T* t)
	{
		StaticAssertCompleteType<T> ErrorTypeIsIncomplete;
		(void)ErrorTypeIsIncomplete;
		delete t;
	}

}

#endif
