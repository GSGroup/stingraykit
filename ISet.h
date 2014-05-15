#ifndef STINGRAY_TOOLKIT_ISET_H
#define STINGRAY_TOOLKIT_ISET_H


#include <stingray/toolkit/ICollection.h>


#define TOOLKIT_DECLARE_SET(ClassName) \
		typedef stingray::ISet<ClassName> ClassName##Set; \
		TOOLKIT_DECLARE_PTR(ClassName##Set); \
		TOOLKIT_DECLARE_COLLECTION(ClassName)

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename T >
	struct ISet : public virtual ICollection<T>, public virtual IReversableEnumerable<T>
	{
		typedef T	ValueType;

		virtual ~ISet() { }

		virtual void Add(const ValueType& value) = 0;
		virtual void Clear() = 0;
		virtual bool Contains(const ValueType& value) const = 0;
		virtual void Remove(const ValueType& value) = 0;
		virtual bool TryRemove(const ValueType& value) = 0;
	};

	/** @} */

}


#endif
