#ifndef STINGRAYKIT_COLLECTION_MAPDICTIONARY_H
#define STINGRAYKIT_COLLECTION_MAPDICTIONARY_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumeratorFromStlContainer.h>
#include <stingraykit/collection/EnumeratorWrapper.h>
#include <stingraykit/collection/ForEach.h>
#include <stingraykit/collection/IDictionary.h>
#include <stingraykit/collection/KeyNotFoundExceptionCreator.h>

#include <algorithm>
#include <map>
#include <vector>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename KeyType_, typename ValueType_ , typename CompareType_ = std::less<KeyType_> >
	class MapDictionary : public virtual IDictionary<KeyType_, ValueType_>
	{
	public:
		typedef KeyType_									KeyType;
		typedef ValueType_									ValueType;
		typedef CompareType_								CompareType;

		typedef KeyValuePair<KeyType, ValueType>			PairType;
		typedef std::map<KeyType, ValueType, CompareType>	MapType;
		STINGRAYKIT_DECLARE_PTR(MapType);

	private:
		struct Holder
		{
			MapTypePtr		Map;
			Holder(const MapTypePtr& map) : Map(map) { }
		};
		STINGRAYKIT_DECLARE_PTR(Holder);

		struct ReverseEnumerable : public virtual IEnumerable<PairType>
		{
			HolderPtr		_holder;

			ReverseEnumerable(const HolderPtr& holder) : _holder(holder) { }

			virtual shared_ptr<IEnumerator<PairType> > GetEnumerator() const
			{ return make_shared<EnumeratorWrapper<typename MapType::value_type, PairType> >(EnumeratorFromStlIterators(_holder->Map->rbegin(), _holder->Map->rend(), _holder)); }
		};

	private:
		MapTypePtr				_map;
		mutable HolderWeakPtr	_mapEnumeratorHolder;

	public:
		MapDictionary() : _map(new MapType)
		{ }

		MapDictionary(const MapDictionary& other) : _map(new MapType(*other._map))
		{ }

		MapDictionary(shared_ptr<IEnumerable<PairType> > enumerable) : _map(new MapType)
		{
			STINGRAYKIT_REQUIRE_NOT_NULL(enumerable);
			FOR_EACH(const PairType p IN enumerable)
				Set(p.Key, p.Value);
		}

		MapDictionary(shared_ptr<IEnumerator<PairType> > enumerator) : _map(new MapType)
		{
			STINGRAYKIT_REQUIRE_NOT_NULL(enumerator);
			FOR_EACH(const PairType p IN enumerator)
				Set(p.Key, p.Value);
		}

		MapDictionary& operator =(const MapDictionary& other)
		{ _map.reset(new MapType(*other._map)); return *this; }

		virtual int GetCount() const { return _map->size(); }
		virtual bool IsEmpty() const { return _map->empty(); }

		virtual bool TryGet(const KeyType& key, ValueType& outValue) const
		{
			typename MapType::const_iterator it = _map->find(key);
			if (it != _map->end())
			{
				outValue = it->second;
				return true;
			}
			else
				return false;
		}

		virtual ValueType Get(const KeyType& key) const
		{
			typename MapType::const_iterator it = _map->find(key);
			STINGRAYKIT_CHECK(it != _map->end(), CreateKeyNotFoundException(key));
			return it->second;
		}

		virtual void Set(const KeyType& key, const ValueType& value)
		{
			CopyOnWrite();
			typename MapType::iterator it = _map->find(key);
			if (it != _map->end())
				it->second = value;
			else
				_map->insert(std::make_pair(key, value));
		}

		virtual void Remove(const KeyType& key)
		{ CopyOnWrite(); _map->erase(key); }

		virtual bool ContainsKey(const KeyType& key) const
		{ return _map->find(key) != _map->end(); }

		virtual shared_ptr<IEnumerator<PairType> > GetEnumerator() const
		{
			shared_ptr<Holder> map_enumerator_holder = _mapEnumeratorHolder.lock();
			if (!map_enumerator_holder)
				_mapEnumeratorHolder = (map_enumerator_holder = make_shared<Holder>(_map));

			return make_shared<EnumeratorWrapper<typename MapType::value_type, PairType> >(EnumeratorFromStlContainer(*_map, map_enumerator_holder));
		}

		virtual shared_ptr<IEnumerable<PairType> > Reverse() const
		{
			shared_ptr<Holder> map_enumerator_holder = _mapEnumeratorHolder.lock();
			if (!map_enumerator_holder)
				_mapEnumeratorHolder = (map_enumerator_holder = make_shared<Holder>(_map));

			return make_shared<ReverseEnumerable>(map_enumerator_holder);
		}

		virtual void Clear()
		{
			CopyOnWrite();
			_map->clear();
		}


	private:
		void CopyOnWrite()
		{
			if (_mapEnumeratorHolder.lock())
			{
				_map.reset(new MapType(*_map));
				_mapEnumeratorHolder.reset();
			}
		}
	};

	/** @} */

}


#endif
