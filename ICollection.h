#ifndef __GS_DVRLIB_TOOLKIT_ICOLLECTION_H__
#define __GS_DVRLIB_TOOLKIT_ICOLLECTION_H__


#include <dvrlib/toolkit/IEnumerable.h>

#define TOOLKIT_DECLARE_COLLECTION(ClassName) \
		typedef dvrlib::ICollection<ClassName>				ClassName##Collection; \
		TOOLKIT_DECLARE_PTR(ClassName##Collection); \
		TOOLKIT_DECLARE_ENUMERABLE(ClassName)

namespace dvrlib
{

	
	template < typename T >
	struct ICollection : public virtual IEnumerable<T>
	{
		virtual ~ICollection() { }

		virtual size_t GetCount() const = 0;
	};


}


#endif
