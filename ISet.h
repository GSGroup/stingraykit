#ifndef STINGRAY_TOOLKIT_ISET_H
#define STINGRAY_TOOLKIT_ISET_H


#include <stingray/settings/IsSerializable.h>
#include <stingray/settings/Serialization.h>
#include <stingray/toolkit/IList.h>
//included here for SerializableList which is the same serialization helper needed here


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
	struct ISet : public virtual ICollection<T>, public virtual IReversableEnumerable<T>, public Detail::SerializableList<ISet<T>, T >
	{
		typedef T	ValueType;

		virtual ~ISet() { }

		virtual void Add(const ValueType& value) = 0;
		virtual void Clear() = 0;
		virtual bool Contains(const ValueType& value) const = 0;
		virtual void Remove(const ValueType& value) = 0;
	};

	/** @} */

}


#endif
