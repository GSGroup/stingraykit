#ifndef STINGRAY_TOOLKIT_ILIST_H
#define STINGRAY_TOOLKIT_ILIST_H


#include <stingray/toolkit/ICollection.h>


#define TOOLKIT_DECLARE_LIST(ClassName) \
		typedef stingray::IList<ClassName>				ClassName##List; \
		TOOLKIT_DECLARE_PTR(ClassName##List); \
		TOOLKIT_DECLARE_COLLECTION(ClassName)

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename T >
	struct IReadonlyList : public virtual ICollection<T>, public virtual IReversableEnumerable<T>
	{
		typedef T	ValueType;

		virtual ~IReadonlyList() { }

		virtual ValueType Get(int index) const = 0;
		virtual int IndexOf(const ValueType& value) const = 0;

		virtual bool Contains(const ValueType& value) const
		{
			return IndexOf(value) != -1;
		}

		virtual bool TryGet(int index, ValueType& value) const
		{
			if (index >= 0 && index < this->GetCount())
			{
				value = Get(index);
				return true;
			}

			return false;
		}
	};


	template < typename T >
	struct InheritsIReadonlyList : public Inherits1ParamTemplate<T, IReadonlyList>
	{ };


	template < typename T >
	struct IList : public virtual IReadonlyList<T>
	{
		typedef T	ValueType;

		virtual ~IList() { }

		virtual void Add(const ValueType& value) = 0;
		virtual void Set(int index, const ValueType& value) = 0;
		virtual void Insert(int index, const ValueType& value) = 0;
		virtual void RemoveAt(int index) = 0;

		virtual void Remove(const ValueType& value)
		{
			int index = this->IndexOf(value);
			if (index != -1)
				RemoveAt(index);
		}

		virtual void Clear()
		{
			while (this->GetCount() != 0)
				Remove(this->Get(0));
		}
	};


	template < typename T >
	struct InheritsIList : public Inherits1ParamTemplate<T, IList>
	{ };

	/** @} */

}


#endif
