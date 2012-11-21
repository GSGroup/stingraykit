#ifndef STINGRAY_TOOLKIT_MAPDICTIONARY_H
#define STINGRAY_TOOLKIT_MAPDICTIONARY_H


#include <map>
#include <vector>
#include <algorithm>

#include <stingray/toolkit/EnumeratorFromStlContainer.h>
#include <stingray/toolkit/IDictionary.h>
#include <stingray/toolkit/StringUtils.h>


namespace stingray
{

	namespace Detail
	{
		//TOOLKIT_DECLARE_METHOD_CHECK(ToString);

		template
		<
			typename KeyType,
			bool HasToString = SameType<std::string, KeyType>::Value || HasMethod_ToString<KeyType>::Value
		>
		struct KeyNotFoundExceptionFactory
		{ static KeyNotFoundException Create(const KeyType& key) { return KeyNotFoundException(StringBuilder() % key); } };

		template < typename KeyType >
		struct KeyNotFoundExceptionFactory<KeyType, false>
		{ static KeyNotFoundException Create(const KeyType& key) { return KeyNotFoundException(); } };
	}

	template < typename KeyType_, typename ValueType_ >
	class MapDictionary : public virtual IDictionary<KeyType_, ValueType_>
	{
	public:
		typedef KeyType_							KeyType;
		typedef ValueType_							ValueType;
		typedef KeyValuePair<KeyType, ValueType>	PairType;
		typedef std::map<KeyType, ValueType>		MapType;

	private:
		MapType		_map;

	public:
		virtual int GetCount() const { return _map.size(); }

		virtual ValueType Get(const KeyType& key) const
		{
			typename MapType::const_iterator it = _map.find(key);
			TOOLKIT_CHECK(it != _map.end(), Detail::KeyNotFoundExceptionFactory<KeyType>::Create(key));
			return it->second;
		}

		virtual void Set(const KeyType& key, const ValueType& value)
		{
			typename MapType::iterator it = _map.find(key);
			if (it != _map.end())
				it->second = value;
			else
				_map.insert(std::make_pair(key, value));
		}

		virtual void Remove(const KeyType& key)
		{
			typename MapType::iterator it = _map.find(key);
			TOOLKIT_CHECK(it != _map.end(), Detail::KeyNotFoundExceptionFactory<KeyType>::Create(key));
			_map.erase(it);
		}

		virtual bool ContainsKey(const KeyType& key) const
		{ return _map.find(key) != _map.end(); }

		virtual shared_ptr<IEnumerator<PairType> > GetEnumerator() const
		{
			shared_ptr<std::vector<PairType> > result(new std::vector<PairType>);
			result->reserve(_map.size());
			for (typename MapType::const_iterator it = _map.begin(); it != _map.end(); ++it)
				result->push_back(PairType(it->first, it->second));
			return EnumeratorFromStlContainer(*result, result);
		}

		virtual shared_ptr<IEnumerable<PairType> > Reverse() const
		{
			shared_ptr<std::vector<PairType> > result(new std::vector<PairType>);
			result->reserve(_map.size());
			for (typename MapType::const_iterator it = _map.begin(); it != _map.end(); ++it)
				result->push_back(PairType(it->first, it->second));
			std::reverse(result->begin(), result->end());
			return EnumerableFromStlContainer(*result, result);
		}
	};


}


#endif
