#ifndef STINGRAYKIT_COLLECTION_MAPDICTIONARY_H
#define STINGRAYKIT_COLLECTION_MAPDICTIONARY_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/EnumeratorWrapper.h>
#include <stingraykit/collection/ForEach.h>
#include <stingraykit/collection/IDictionary.h>
#include <stingraykit/collection/KeyNotFoundExceptionCreator.h>
#include <stingraykit/collection/flat_map.h>
#include <stingraykit/compare/comparers.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template <
			typename KeyType_,
			typename ValueType_,
			typename CompareType_ = comparers::Less,
			template <class, class, class, class> class MapType_ = std::map,
			typename AllocatorType_ = std::allocator<std::pair<const KeyType_, ValueType_> >
			>
	class MapDictionary : public virtual IDictionary<KeyType_, ValueType_>
	{
	public:
		typedef KeyType_									KeyType;
		typedef ValueType_									ValueType;
		typedef CompareType_								CompareType;
		typedef AllocatorType_								AllocatorType;

		typedef KeyValuePair<KeyType, ValueType>							PairType;
		typedef MapType_<KeyType, ValueType, CompareType, AllocatorType>	MapType;
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
			{ return WrapMapEnumerator(EnumeratorFromStlIterators(_holder->Map->rbegin(), _holder->Map->rend(), _holder)); }
		};

	private:
		MapTypePtr				_map;
		mutable HolderWeakPtr	_mapHolder;

	public:
		MapDictionary()
			:	_map(make_shared<MapType>())
		{ }

		MapDictionary(const MapDictionary& other)
		{ CopyMap(other._map); }

		MapDictionary(shared_ptr<IEnumerable<PairType> > enumerable)
			:	_map(make_shared<MapType>())
		{
			STINGRAYKIT_REQUIRE_NOT_NULL(enumerable);
			FOR_EACH(const PairType p IN enumerable)
				Set(p.Key, p.Value);
		}

		MapDictionary(shared_ptr<IEnumerator<PairType> > enumerator)
			:	_map(make_shared<MapType>())
		{
			STINGRAYKIT_REQUIRE_NOT_NULL(enumerator);
			FOR_EACH(const PairType p IN enumerator)
				Set(p.Key, p.Value);
		}

		MapDictionary& operator =(const MapDictionary& other)
		{ CopyMap(other._map); return *this; }

		virtual shared_ptr<IEnumerator<PairType> > GetEnumerator() const
		{ return WrapMapEnumerator(EnumeratorFromStlContainer(*_map, GetMapHolder())); }

		virtual shared_ptr<IEnumerable<PairType> > Reverse() const
		{ return make_shared<ReverseEnumerable>(GetMapHolder()); }

		virtual size_t GetCount() const
		{ return _map->size(); }

		virtual bool IsEmpty() const
		{ return _map->empty(); }

		virtual bool ContainsKey(const KeyType& key) const
		{ return _map->find(key) != _map->end(); }

		virtual shared_ptr<IEnumerator<PairType> > Find(const KeyType& key) const
		{
			typedef typename MapType::const_iterator cit;

			cit it = _map->find(key);
			if (it == _map->end())
				return MakeEmptyEnumerator();

			return WrapMapEnumerator(EnumeratorFromStlIterators(it, cit(_map->end()), GetMapHolder()));
		}

		virtual shared_ptr<IEnumerator<PairType> > ReverseFind(const KeyType& key) const
		{
			typedef typename MapType::const_reverse_iterator cri;

			typename MapType::const_iterator it = _map->find(key);
			if (it == _map->end())
				return MakeEmptyEnumerator();

			return WrapMapEnumerator(EnumeratorFromStlIterators(cri(++it), cri(_map->rend()), GetMapHolder()));
		}

		virtual ValueType Get(const KeyType& key) const
		{
			typename MapType::const_iterator it = _map->find(key);
			STINGRAYKIT_CHECK(it != _map->end(), CreateKeyNotFoundException(key));
			return it->second;
		}

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

		virtual bool TryRemove(const KeyType& key)
		{
			typename MapType::iterator it = _map->find(key);
			if (it == _map->end())
				return false;

			CopyOnWrite();
			_map->erase(key);
			return true;
		}

		virtual size_t RemoveWhere(const function<bool (const KeyType&, const ValueType&)>& pred)
		{
			CopyOnWrite();
			size_t ret = 0;
			for (typename MapType::iterator it = _map->begin(); it != _map->end(); )
			{
				const typename MapType::iterator cur = it++;
				if (!pred(cur->first, cur->second))
					continue;

				_map->erase(cur);
				++ret;
			}
			return ret;
		}

		virtual void Clear()
		{
			CopyOnWrite();
			_map->clear();
		}

	private:
		void CopyMap(const MapTypePtr& map)
		{
			_map = make_shared<MapType>(*map);
			_mapHolder.reset();
		}

		HolderPtr GetMapHolder() const
		{
			HolderPtr mapHolder = _mapHolder.lock();

			if (!mapHolder)
				_mapHolder = (mapHolder = make_shared<Holder>(_map));

			return mapHolder;
		}

		void CopyOnWrite()
		{
			if (!_mapHolder.expired())
				CopyMap(_map);
		}

		static shared_ptr<IEnumerator<PairType> > WrapMapEnumerator(const shared_ptr<IEnumerator<typename MapType::value_type> >& mapEnumerator)
		{ return make_shared<EnumeratorWrapper<typename MapType::value_type, PairType> >(mapEnumerator); }
	};

	template <
			typename KeyType,
			typename ValueType,
			typename CompareType = comparers::Less,
			typename AllocatorType = typename flat_map<KeyType, ValueType, CompareType>::allocator_type
			>
	struct FlatMapDictionary
	{ typedef MapDictionary<KeyType, ValueType, CompareType, flat_map, AllocatorType>		Type; };

	/** @} */

}

#endif
