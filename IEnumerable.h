#ifndef __GS_DVRLIB_TOOLKIT_IENUMERABLE_H__
#define __GS_DVRLIB_TOOLKIT_IENUMERABLE_H__


#include <dvrlib/toolkit/IEnumerator.h>

#define TOOLKIT_DECLARE_ENUMERABLE(ClassName) \
		typedef dvrlib::IEnumerable<ClassName>				ClassName##Enumerable; \
		TOOLKIT_DECLARE_PTR(ClassName##Enumerable); \
		TOOLKIT_DECLARE_ENUMERATOR(ClassName)

namespace dvrlib
{

	
	template < typename T >
	struct IEnumerable
	{
		typedef T ItemType;

		virtual ~IEnumerable() { }

		virtual shared_ptr<IEnumerator<T> > GetEnumerator() = 0;
	};


}


#endif
