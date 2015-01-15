#ifndef STINGRAYKIT_COLLECTION_ICOLLECTION_H
#define STINGRAYKIT_COLLECTION_ICOLLECTION_H


#include <stingraykit/collection/IEnumerable.h>

#define STINGRAYKIT_DECLARE_COLLECTION(ClassName) \
		typedef stingray::ICollection<ClassName>				ClassName##Collection; \
		STINGRAYKIT_DECLARE_PTR(ClassName##Collection); \
		STINGRAYKIT_DECLARE_ENUMERABLE(ClassName)

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
