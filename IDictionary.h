#ifndef STINGRAY_TOOLKIT_IDICTIONARY_H
#define STINGRAY_TOOLKIT_IDICTIONARY_H


#include <stingray/toolkit/IEnumerable.h>


namespace stingray
{

	struct KeyNotFoundException : public stingray::Exception
	{
		KeyNotFoundException() : stingray::Exception("Key not found!") { }
		KeyNotFoundException(const std::string& keyStr) : stingray::Exception("Key '" + keyStr + "' not found!") { }
		virtual ~KeyNotFoundException() throw() { }
	};

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
	};

	template < typename KeyType_, typename ValueType_ >
	struct IDictionary : public IEnumerable<KeyValuePair<KeyType_, ValueType_> >
	{
		typedef KeyType_							KeyType;
		typedef ValueType_							ValueType;
		typedef KeyValuePair<KeyType, ValueType>	PairType;

		virtual ~IDictionary() { }

		virtual ValueType Get(const KeyType& key) const = 0;
		virtual void Set(const KeyType& key, const ValueType& value) = 0;

		virtual void Remove(const KeyType& key) = 0;

		virtual bool ContainsKey(const KeyType& key) const = 0;

		virtual bool TryGet(const KeyType& key, ValueType& outValue) const
		{
			if (!ContainsKey(key))
				return false;
			outValue = Get(key);
			return true;
		}
	};

}


#endif
