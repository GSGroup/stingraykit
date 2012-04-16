#ifndef __GS_DVRLIB_TOOLKIT_IENUMERABLE_H__
#define __GS_DVRLIB_TOOLKIT_IENUMERABLE_H__


#include <stingray/toolkit/IEnumerator.h>

#define TOOLKIT_DECLARE_ENUMERABLE(ClassName) \
		typedef stingray::IEnumerable<ClassName>				ClassName##Enumerable; \
		TOOLKIT_DECLARE_PTR(ClassName##Enumerable); \
		TOOLKIT_DECLARE_ENUMERATOR(ClassName)

namespace stingray
{


	template < typename T >
	struct IEnumerable
	{
		typedef T ItemType;

		virtual ~IEnumerable() { }

		virtual shared_ptr<IEnumerator<T> > GetEnumerator() const = 0;
	};


}


#endif
