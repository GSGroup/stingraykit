#ifndef STINGRAY_TOOLKIT_COLLECTIONS_ICOLLECTION_H
#define STINGRAY_TOOLKIT_COLLECTIONS_ICOLLECTION_H


#include <stingray/toolkit/collection/IEnumerable.h>

#define TOOLKIT_DECLARE_COLLECTION(ClassName) \
		typedef stingray::ICollection<ClassName>				ClassName##Collection; \
		TOOLKIT_DECLARE_PTR(ClassName##Collection); \
		TOOLKIT_DECLARE_ENUMERABLE(ClassName)

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename T >
	struct ICollection : public virtual IEnumerable<T>
	{
		virtual ~ICollection() { }

		virtual int GetCount() const = 0;
		virtual bool IsEmpty() const = 0;
	};

	/** @} */


}


#endif
