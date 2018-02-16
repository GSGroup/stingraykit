#ifndef STINGRAYKIT_COLLECTION_MULTIMAPDICTIONARY_H
#define STINGRAYKIT_COLLECTION_MULTIMAPDICTIONARY_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/EnumeratorWrapper.h>
#include <stingraykit/collection/ForEach.h>
#include <stingraykit/collection/IMultiDictionary.h>
#include <stingraykit/collection/KeyNotFoundExceptionCreator.h>
#include <stingraykit/compare/comparers.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename KeyType_, typename ValueType_, typename KeyCompareType_ = comparers::Less, typename ValueCompareType_ = comparers::Equals >
	class MultiMapDictionary : public virtual IMultiDictionary<KeyType_, ValueType_>
	{
	public:
		typedef KeyType_									KeyType;
		typedef ValueType_									ValueType;
		typedef KeyCompareType_								KeyCompareType;
		typedef ValueCompareType_							ValueCompareType;

		typedef KeyValuePair<KeyType, ValueType>							PairType;
		typedef std::multimap<KeyType, ValueType, KeyCompareType>			MapType;
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
		MultiMapDictionary()
			:	_map(make_shared<MapType>())
		{ }

		MultiMapDictionary(const MultiMapDictionary& other)
		{ CopyMap(other._map); }

		MultiMapDictionary(shared_ptr<IEnumerable<PairType> > enumerable)
			:	_map(make_shared<MapType>())
		{
			STINGRAYKIT_REQUIRE_NOT_NULL(enumerable);
			FOR_EACH(const PairType p IN enumerable)
				Set(p.Key, p.Value);
		}

		MultiMapDictionary(shared_ptr<IEnumerator<PairType> > enumerator)
			:	_map(make_shared<MapType>())
		{
			STINGRAYKIT_REQUIRE_NOT_NULL(enumerator);
			FOR_EACH(const PairType p IN enumerator)
				Set(p.Key, p.Value);
		}

		MultiMapDictionary& operator =(const MultiMapDictionary& other)
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

		virtual size_t CountKey(const KeyType& key) const
		{ return _map->count(key); }

		virtual shared_ptr<IEnumerator<PairType> > Find(const KeyType& key) const
		{
			typedef typename MapType::const_iterator cit;

			cit it = _map->lower_bound(key);
			if (it == _map->end() || KeyCompareType()(key, it->first))
				return MakeEmptyEnumerator();

			return WrapMapEnumerator(EnumeratorFromStlIterators(it, cit(_map->end()), GetMapHolder()));
		}

		virtual shared_ptr<IEnumerator<PairType> > ReverseFind(const KeyType& key) const
		{
			typedef typename MapType::const_reverse_iterator cri;

			typename MapType::const_iterator it = _map->upper_bound(key);
			if (it == _map->end() || KeyCompareType()(cri(it)->first, key))
				return MakeEmptyEnumerator();

			return WrapMapEnumerator(EnumeratorFromStlIterators(cri(it), cri(_map->rend()), GetMapHolder()));
		}

		virtual ValueType GetFirst(const KeyType& key) const
		{
			typename MapType::const_iterator it = _map->lower_bound(key);
			STINGRAYKIT_CHECK(it != _map->end() && !KeyCompareType()(key, it->first), CreateKeyNotFoundException(key));
			return it->second;
		}

		virtual bool TryGetFirst(const KeyType& key, ValueType& outValue) const
		{
			typename MapType::const_iterator it = _map->lower_bound(key);
			if (it == _map->end() || KeyCompareType()(key, it->first))
				return false;

			outValue = it->second;
			return true;
		}

		virtual shared_ptr<IEnumerator<PairType> > GetAll(const KeyType& key) const
		{
			typedef typename MapType::const_iterator cit;
			std::pair<cit, cit> range = _map->equal_range(key);
			if (range.first == range.second)
				return MakeEmptyEnumerator();

			return WrapMapEnumerator(EnumeratorFromStlIterators(range.first, range.second, GetMapHolder()));
		}

		virtual void Set(const KeyType& key, const ValueType& value)
		{ CopyOnWrite(); _map->insert(std::make_pair(key, value)); }

		virtual void RemoveFirst(const KeyType& key, const optional<ValueType>& value = null)
		{ DoRemoveFirst(key, value); }

		virtual bool TryRemoveFirst(const KeyType& key, const optional<ValueType>& value = null)
		{ return DoRemoveFirst(key, value); }

		virtual size_t RemoveAll(const KeyType& key)
		{
			CopyOnWrite();
			return _map->erase(key);
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
		{ CopyOnWrite(); _map->clear(); }

	private:
		bool DoRemoveFirst(const KeyType& key, const optional<ValueType>& value = null)
		{
			CopyOnWrite();
			typedef typename MapType::iterator mit;
			std::pair<mit, mit> range = _map->equal_range(key);
			for (mit it = range.first; it != range.second; ++it)
			{
				if (value && !ValueCompareType()(*value, it->second))
					continue;

				_map->erase(it);
				return true;
			}
			return false;
		}

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
			if (_mapHolder.lock())
				CopyMap(_map);
		}

		static shared_ptr<IEnumerator<PairType> > WrapMapEnumerator(const shared_ptr<IEnumerator<typename MapType::value_type> >& mapEnumerator)
		{ return make_shared<EnumeratorWrapper<typename MapType::value_type, PairType> >(mapEnumerator); }
	};

	/** @} */

}

#endif
