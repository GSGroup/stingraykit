#ifndef STINGRAY_TOOLKIT_ILIST_H
#define STINGRAY_TOOLKIT_ILIST_H


#include <stingray/settings/IsSerializable.h>
#include <stingray/settings/Serialization.h>
#include <stingray/toolkit/ICollection.h>


#define TOOLKIT_DECLARE_LIST(ClassName) \
		typedef stingray::IList<ClassName>				ClassName##List; \
		TOOLKIT_DECLARE_PTR(ClassName##List); \
		TOOLKIT_DECLARE_COLLECTION(ClassName)

namespace stingray
{

	namespace Detail
	{
		template < typename T, typename ValueType_, bool IsSerializable_ = IsSerializable<ValueType_>::Value >
		struct SerializableList : public virtual ISerializable
		{
			virtual ~SerializableList() { }

			virtual void Serialize(ObjectOStream & ar) const
			{
				const T* inst = static_cast<const T*>(this);
				std::vector<ValueType_> v;
				v.reserve(inst->GetCount());
				FOR_EACH(ValueType_ val IN inst->GetEnumerator())
					v.push_back(val);
				ar.Serialize("data", v);
			}

			virtual void Deserialize(ObjectIStream & ar)
			{
				T* inst = static_cast<T*>(this);
				std::vector<ValueType_> v;
				ar.Deserialize("data", v);
				inst->Clear();
				for (typename std::vector<ValueType_>::const_iterator it = v.begin(); it != v.end(); ++it)
					inst->Add(*it);
			}
		};

		template < typename T, typename ValueType_ >
		struct SerializableList<T, ValueType_, false>
		{
			virtual ~SerializableList() { }
		};
	}
	
	template < typename T >
	struct IList : public virtual ICollection<T>, public virtual IReversableEnumerable<T>, public Detail::SerializableList<IList<T>, T >
	{
		typedef T	ValueType;

		virtual ~IList() { }
		
		virtual void Add(const ValueType& value) = 0;
		virtual ValueType Get(int index) const = 0;
		virtual void Set(int index, const ValueType& value) = 0;
		virtual int IndexOf(const ValueType& value) const = 0;
		virtual void Insert(int index, const ValueType& value) = 0;
		virtual void RemoveAt(int index) = 0;

		virtual void Remove(const ValueType& value)
		{
			int index = IndexOf(value);
			if (index != -1)
				RemoveAt(index);
		}

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

		virtual void Clear()
		{
			while (this->GetCount() != 0)
				Remove(Get(0));
		}
	};


}


#endif
