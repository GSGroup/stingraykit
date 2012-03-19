#ifndef __GS_DVRLIB_TOOLKIT_ICOLLECTION_H__
#define __GS_DVRLIB_TOOLKIT_ICOLLECTION_H__


#include <stingray/toolkit/IEnumerable.h>

#define TOOLKIT_DECLARE_COLLECTION(ClassName) \
		typedef stingray::ICollection<ClassName>				ClassName##Collection; \
		TOOLKIT_DECLARE_PTR(ClassName##Collection); \
		TOOLKIT_DECLARE_ENUMERABLE(ClassName)

namespace stingray
{

	
	template < typename T >
	struct ICollection : public virtual IEnumerable<T>
	{
		virtual ~ICollection() { }

		virtual size_t GetCount() const = 0;
	};


}


#endif
