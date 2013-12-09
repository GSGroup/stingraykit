#ifndef STINGRAY_TOOLKIT_IDICTIONARY_H
#define STINGRAY_TOOLKIT_IDICTIONARY_H


#include <utility>

#include <stingray/settings/IsSerializable.h>
#include <stingray/settings/Serialization.h>
#include <stingray/toolkit/ICollection.h>
#include <stingray/toolkit/IEnumerable.h>
#include <stingray/toolkit/KeyNotFoundExceptionCreator.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename KeyType_, typename ValueType_ >
	struct KeyValuePair
	{
		typedef KeyType_	KeyType;
		typedef ValueType_	ValueType;

		KeyType		Key;
		ValueType	Value;

		KeyValuePair(const KeyType& key, const ValueType& value)
			: Key(key), Value(value)
		{ }

		KeyValuePair(const std::pair<KeyType, ValueType>& pair)
			: Key(pair.first), Value(pair.second)
		{ }

		KeyType GetKey() const			{ return Key; }
		ValueType GetValue() const		{ return Value; }

		std::string ToString() const	{ return StringBuilder() % Key % " -> " % Value; }
	};

	namespace Detail
	{
		template < typename T, typename KeyType_, typename ValueType_, bool IsSerializable_ = IsSerializable<KeyType_>::Value && IsSerializable<ValueType_>::Value >
		struct SerializableDictionary : public virtual ISerializable
		{
			virtual ~SerializableDictionary() { }

			virtual void Serialize(ObjectOStream & ar) const
			{
				typedef KeyValuePair<KeyType_, ValueType_>	PairType;
				const T* inst = static_cast<const T*>(this);
				std::map<KeyType_, ValueType_> m;
				FOR_EACH(PairType p IN inst->GetEnumerator())
					m.insert(std::make_pair(p.Key, p.Value));
				ar.Serialize("data", m);
			}

			virtual void Deserialize(ObjectIStream & ar)
			{
				T* inst = static_cast<T*>(this);
				std::map<KeyType_, ValueType_> m;
				ar.Deserialize("data", m);
				inst->Clear();
				for (typename std::map<KeyType_, ValueType_>::const_iterator it = m.begin(); it != m.end(); ++it)
					inst->Set(it->first, it->second);
			}
		};

		template < typename T, typename KeyType_, typename ValueType_ >
		struct SerializableDictionary<T, KeyType_, ValueType_, false>
		{
			virtual ~SerializableDictionary() { }
		};
	}

	template < typename KeyType_, typename ValueType_ >
	struct IReadonlyDictionary :
		public virtual ICollection<KeyValuePair<KeyType_, ValueType_> >,
		public virtual IReversableEnumerable<KeyValuePair<KeyType_, ValueType_> >
	{
		typedef KeyType_							KeyType;
		typedef ValueType_							ValueType;
		typedef KeyValuePair<KeyType, ValueType>	PairType;

		virtual ~IReadonlyDictionary() { }

		virtual ValueType Get(const KeyType& key) const = 0;

		virtual bool ContainsKey(const KeyType& key) const = 0;

		virtual bool TryGet(const KeyType& key, ValueType& outValue) const
		{
			if (!ContainsKey(key))
				return false;
			outValue = Get(key);
			return true;
		}
	};

	template < typename KeyType_, typename ValueType_ >
	struct IDictionary :
		public virtual IReadonlyDictionary<KeyType_, ValueType_>,
		public Detail::SerializableDictionary<IDictionary<KeyType_, ValueType_>, KeyType_, ValueType_ >
	{
		typedef KeyType_							KeyType;
		typedef ValueType_							ValueType;
		typedef KeyValuePair<KeyType, ValueType>	PairType;

		virtual ~IDictionary() { }

		virtual void Set(const KeyType& key, const ValueType& value) = 0;

		virtual void Remove(const KeyType& key) = 0;

		virtual bool TryRemove(const KeyType& key)
		{
			if (!this->ContainsKey(key))
				return false;
			Remove(key);
			return true;
		}

		virtual void Clear()
		{
			while (this->GetCount() != 0)
				Remove(this->GetEnumerator()->Get().Key);
		}
	};


	template<typename EnumeratorType>
	shared_ptr<IEnumerator<typename EnumeratorType::PairType::KeyType> > KeysEnumerator(const shared_ptr<EnumeratorType>& enumerator)
	{
		typedef typename EnumeratorType::PairType PairType;
		return WrapEnumerator(enumerator, &PairType::GetKey);
	}

	template<typename EnumerableType>
	shared_ptr<IEnumerable<typename EnumerableType::PairType::KeyType> > KeysEnumerable(const shared_ptr<EnumerableType>& enumerable)
	{
		typedef typename EnumerableType::PairType PairType;
		return WrapEnumerable(enumerable, &PairType::GetKey);
	}
	/** @} */

}


#endif
