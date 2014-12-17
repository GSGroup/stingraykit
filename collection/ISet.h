#ifndef STINGRAY_TOOLKIT_COLLECTIONS_ISET_H
#define STINGRAY_TOOLKIT_COLLECTIONS_ISET_H


#include <stingray/toolkit/collection/ICollection.h>


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
	struct IReadonlySet : public virtual ICollection<T>, public virtual IReversableEnumerable<T>
	{
		typedef T	ValueType;

		virtual ~IReadonlySet() { }

		virtual bool Contains(const ValueType& value) const = 0;
	};


	template < typename T >
	struct ISet : public virtual IReadonlySet<T>
	{
		typedef T	ValueType;

		virtual ~ISet() { }

		virtual void Add(const ValueType& value) = 0;
		virtual void Clear() = 0;
		virtual void Remove(const ValueType& value) = 0;
		virtual bool TryRemove(const ValueType& value) = 0;
	};


	template < typename T >
	struct InheritsIReadonlySet : public Inherits1ParamTemplate<T, IReadonlySet>
	{ };

	/** @} */

}


#endif
