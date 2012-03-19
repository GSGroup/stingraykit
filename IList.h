#ifndef __GS_DVRLIB_TOOLKIT_ILIST_H__
#define __GS_DVRLIB_TOOLKIT_ILIST_H__


#include <dvrlib/toolkit/ICollection.h>


#define TOOLKIT_DECLARE_LIST(ClassName) \
		typedef dvrlib::IList<ClassName>				ClassName##List; \
		TOOLKIT_DECLARE_PTR(ClassName##List); \
		TOOLKIT_DECLARE_COLLECTION(ClassName)

namespace dvrlib
{

	
	template < typename T >
	struct IList : public virtual ICollection<T>
	{
		typedef T											ValueType;
		typedef typename GetConstReferenceType<T>::ValueT	ConstTRef;

		virtual ~IList() { }
		
		virtual void Add(ConstTRef obj) = 0;
		virtual ValueType Get(size_t index) const = 0;
		virtual void Remove(ConstTRef obj) = 0;
	};


}


#endif
