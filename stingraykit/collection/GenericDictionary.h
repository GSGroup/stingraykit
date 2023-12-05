#ifndef STINGRAYKIT_COLLECTION_GENERICDICTIONARY_H
#define STINGRAYKIT_COLLECTION_GENERICDICTIONARY_H

// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/ForEach.h>
#include <stingraykit/collection/IDictionary.h>
#include <stingraykit/collection/KeyNotFoundExceptionCreator.h>
#include <stingraykit/function/function.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename MapType_ >
	class GenericDictionary : public virtual IDictionary<typename MapType_::key_type, typename MapType_::mapped_type>
	{
		static_assert(comparers::IsRelationalComparer<typename MapType_::key_compare>::Value, "Expected Relational comparer");

	public:
		using KeyType = typename MapType_::key_type;
		using ValueType = typename MapType_::mapped_type;

		using PairType = KeyValuePair<KeyType, ValueType>;
		using MapType = MapType_;
		STINGRAYKIT_DECLARE_PTR(MapType);

	private:
		struct Holder
		{
			const MapTypePtr		Map;

			explicit Holder(const MapTypePtr& map) : Map(map) { }
		};
		STINGRAYKIT_DECLARE_PTR(Holder);

		class ReverseEnumerable : public virtual IEnumerable<PairType>
		{
		private:
			const HolderPtr			_holder;

		public:
			explicit ReverseEnumerable(const HolderPtr& holder) : _holder(holder) { }

			shared_ptr<IEnumerator<PairType>> GetEnumerator() const override
			{ return WrapMapEnumerator(EnumeratorFromStlIterators(_holder->Map->rbegin(), _holder->Map->rend(), _holder)); }
		};

	private:
		MapTypePtr				_map;
		mutable HolderWeakPtr	_mapHolder;

	public:
		GenericDictionary()
			:	_map(make_shared_ptr<MapType>())
		{ }

		GenericDictionary(const GenericDictionary& other)
		{ CopyMap(other._map); }

		explicit GenericDictionary(const shared_ptr<IEnumerable<PairType>>& enumerable)
			:	GenericDictionary(STINGRAYKIT_REQUIRE_NOT_NULL(enumerable)->GetEnumerator())
		{ }

		explicit GenericDictionary(const shared_ptr<IEnumerator<PairType>>& enumerator)
			:	_map(make_shared_ptr<MapType>())
		{
			STINGRAYKIT_CHECK(enumerator, NullArgumentException("enumerator"));
			FOR_EACH(const PairType pair IN enumerator)
				Set(pair.Key, pair.Value);
		}

		GenericDictionary& operator = (const GenericDictionary& other)
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

		shared_ptr<IEnumerator<PairType>> Find(const KeyType& key) const override
		{
			const auto it = _map->find(key);
			if (it == _map->end())
				return MakeEmptyEnumerator();

			return WrapMapEnumerator(EnumeratorFromStlIterators(it, _map->end(), GetMapHolder()));
		}

		shared_ptr<IEnumerator<PairType>> ReverseFind(const KeyType& key) const override
		{
			auto it = _map->find(key);
			if (it == _map->end())
				return MakeEmptyEnumerator();

			return WrapMapEnumerator(EnumeratorFromStlIterators(typename MapType::const_reverse_iterator(++it), _map->crend(), GetMapHolder()));
		}

		ValueType Get(const KeyType& key) const override
		{
			const auto it = _map->find(key);
			STINGRAYKIT_CHECK(it != _map->end(), CreateKeyNotFoundException(key));
			return it->second;
		}

		bool TryGet(const KeyType& key, ValueType& outValue) const override
		{
			const auto it = _map->find(key);
			if (it != _map->end())
			{
				outValue = it->second;
				return true;
			}
			else
				return false;
		}

		void Set(const KeyType& key, const ValueType& value) override
		{
			CopyOnWrite();
			const auto it = _map->find(key);
			if (it != _map->end())
				it->second = value;
			else
				_map->emplace(key, value);
		}

		void Remove(const KeyType& key) override
		{
			CopyOnWrite();
			_map->erase(key);
		}

		bool TryRemove(const KeyType& key) override
		{
			const auto it = _map->find(key);
			if (it == _map->end())
				return false;

			CopyOnWrite();
			_map->erase(key);
			return true;
		}

		size_t RemoveWhere(const function<bool (const KeyType&, const ValueType&)>& pred) override
		{
			CopyOnWrite();
			size_t ret = 0;
			for (auto it = _map->begin(); it != _map->end(); )
			{
				if (pred(it->first, it->second))
				{
					it = _map->erase(it);
					++ret;
				}
				else
					++it;
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
