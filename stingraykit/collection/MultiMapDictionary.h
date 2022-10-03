#ifndef STINGRAYKIT_COLLECTION_MULTIMAPDICTIONARY_H
#define STINGRAYKIT_COLLECTION_MULTIMAPDICTIONARY_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/ForEach.h>
#include <stingraykit/collection/IMultiDictionary.h>
#include <stingraykit/collection/KeyNotFoundExceptionCreator.h>
#include <stingraykit/collection/iterators.h>
#include <stingraykit/compare/comparers.h>
#include <stingraykit/function/function.h>

#include <map>

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
		using KeyType = KeyType_;
		using ValueType = ValueType_;
		using KeyCompareType = KeyCompareType_;
		using ValueCompareType = ValueCompareType_;

		using PairType = KeyValuePair<KeyType, ValueType>;
		using MapType = std::multimap<KeyType, ValueType, KeyCompareType>;
		STINGRAYKIT_DECLARE_PTR(MapType);

	private:
		struct Holder
		{
			MapTypePtr		Map;

			explicit Holder(const MapTypePtr& map) : Map(map) { }
		};
		STINGRAYKIT_DECLARE_PTR(Holder);

		struct ReverseEnumerable : public virtual IEnumerable<PairType>
		{
			HolderPtr		_holder;

			explicit ReverseEnumerable(const HolderPtr& holder) : _holder(holder) { }

			shared_ptr<IEnumerator<PairType>> GetEnumerator() const override
			{ return WrapMapEnumerator(EnumeratorFromStlIterators(_holder->Map->rbegin(), _holder->Map->rend(), _holder)); }
		};

	private:
		MapTypePtr				_map;
		mutable HolderWeakPtr	_mapHolder;

	public:
		MultiMapDictionary()
			:	_map(make_shared_ptr<MapType>())
		{ }

		MultiMapDictionary(const MultiMapDictionary& other)
		{ CopyMap(other._map); }

		explicit MultiMapDictionary(const shared_ptr<IEnumerable<PairType>>& enumerable)
			:	_map(make_shared_ptr<MapType>())
		{
			STINGRAYKIT_REQUIRE_NOT_NULL(enumerable);
			FOR_EACH(const PairType p IN enumerable)
				Add(p.Key, p.Value);
		}

		explicit MultiMapDictionary(const shared_ptr<IEnumerator<PairType>>& enumerator)
			:	_map(make_shared_ptr<MapType>())
		{
			STINGRAYKIT_REQUIRE_NOT_NULL(enumerator);
			FOR_EACH(const PairType p IN enumerator)
				Add(p.Key, p.Value);
		}

		MultiMapDictionary& operator = (const MultiMapDictionary& other)
		{ CopyMap(other._map); return *this; }

		shared_ptr<IEnumerator<PairType>> GetEnumerator() const override
		{ return WrapMapEnumerator(EnumeratorFromStlContainer(*_map, GetMapHolder())); }

		shared_ptr<IEnumerable<PairType>> Reverse() const override
		{ return make_shared_ptr<ReverseEnumerable>(GetMapHolder()); }

		size_t GetCount() const override
		{ return _map->size(); }

		bool IsEmpty() const override
		{ return _map->empty(); }

		bool ContainsKey(const KeyType& key) const override
		{ return _map->find(key) != _map->end(); }

		size_t CountKey(const KeyType& key) const override
		{ return _map->count(key); }

		shared_ptr<IEnumerator<PairType>> Find(const KeyType& key) const override
		{
			using cit = typename MapType::const_iterator;

			cit it = _map->lower_bound(key);
			if (it == _map->end() || KeyCompareType()(key, it->first))
				return MakeEmptyEnumerator();

			return WrapMapEnumerator(EnumeratorFromStlIterators(it, cit(_map->end()), GetMapHolder()));
		}

		shared_ptr<IEnumerator<PairType>> ReverseFind(const KeyType& key) const override
		{
			using cri = typename MapType::const_reverse_iterator;

			typename MapType::const_iterator it = _map->upper_bound(key);
			if (it == _map->end() || KeyCompareType()(cri(it)->first, key))
				return MakeEmptyEnumerator();

			return WrapMapEnumerator(EnumeratorFromStlIterators(cri(it), cri(_map->rend()), GetMapHolder()));
		}

		ValueType GetFirst(const KeyType& key) const override
		{
			typename MapType::const_iterator it = _map->lower_bound(key);
			STINGRAYKIT_CHECK(it != _map->end() && !KeyCompareType()(key, it->first), CreateKeyNotFoundException(key));
			return it->second;
		}

		bool TryGetFirst(const KeyType& key, ValueType& outValue) const override
		{
			typename MapType::const_iterator it = _map->lower_bound(key);
			if (it == _map->end() || KeyCompareType()(key, it->first))
				return false;

			outValue = it->second;
			return true;
		}

		shared_ptr<IEnumerator<ValueType>> GetAll(const KeyType& key) const override
		{
			using cit = typename MapType::const_iterator;
			std::pair<cit, cit> range = _map->equal_range(key);
			if (range.first == range.second)
				return MakeEmptyEnumerator();

			return EnumeratorFromStlIterators(values_iterator(range.first), values_iterator(range.second), GetMapHolder());
		}

		void Add(const KeyType& key, const ValueType& value) override
		{
			CopyOnWrite();
			_map->insert(std::make_pair(key, value));
		}

		void RemoveFirst(const KeyType& key, const optional<ValueType>& value = null) override
		{ DoRemoveFirst(key, value); }

		bool TryRemoveFirst(const KeyType& key, const optional<ValueType>& value = null) override
		{ return DoRemoveFirst(key, value); }

		size_t RemoveAll(const KeyType& key) override
		{
			CopyOnWrite();
			return _map->erase(key);
		}

		size_t RemoveWhere(const function<bool (const KeyType&, const ValueType&)>& pred) override
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

		void Clear() override
		{
			if (_mapHolder.expired())
				_map->clear();
			else
			{
				_map = make_shared_ptr<MapType>();
				_mapHolder.reset();
			}
		}

	private:
		bool DoRemoveFirst(const KeyType& key, const optional<ValueType>& value = null)
		{
			CopyOnWrite();
			using mit = typename MapType::iterator;
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
			_map = make_shared_ptr<MapType>(*map);
			_mapHolder.reset();
		}

		HolderPtr GetMapHolder() const
		{
			HolderPtr mapHolder = _mapHolder.lock();

			if (!mapHolder)
				_mapHolder = (mapHolder = make_shared_ptr<Holder>(_map));

			return mapHolder;
		}

		void CopyOnWrite()
		{
			if (!_mapHolder.expired())
				CopyMap(_map);
		}

		static shared_ptr<IEnumerator<PairType>> WrapMapEnumerator(const shared_ptr<IEnumerator<typename MapType::value_type>>& mapEnumerator)
		{ return WrapEnumerator(mapEnumerator); }
	};

	/** @} */

}

#endif
