#ifndef STINGRAY_TOOLKIT_ICOLLECTION_H
#define STINGRAY_TOOLKIT_ICOLLECTION_H


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

		virtual int GetCount() const = 0;
		virtual void Clear() = 0;
	};


}


#endif
