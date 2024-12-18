#ifndef STINGRAYKIT_COLLECTION_GENERICDICTIONARY_H
#define STINGRAYKIT_COLLECTION_GENERICDICTIONARY_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/ForEach.h>
#include <stingraykit/collection/IDictionary.h>
#include <stingraykit/collection/KeyExceptionCreator.h>
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

		bool Add(const KeyType& key, const ValueType& value) override
		{ return DoAdd<MapType>(key, value, 0); }

		void Set(const KeyType& key, const ValueType& value) override
		{ DoSet<MapType>(key, value, 0); }

		bool Remove(const KeyType& key) override
		{
			const auto it = _map->find(key);
			if (it == _map->end())
				return false;

			if (CopyOnWrite())
				_map->erase(key);
			else
				_map->erase(it);

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
		template < typename MapType__ >
		auto DoAdd(const KeyType& key, const ValueType& value, int) -> decltype(std::declval<MapType__>().lower_bound(key), bool())
		{
			const auto it = _map->lower_bound(key);
			if (it != _map->end() && !typename MapType__::key_compare()(key, it->first))
				return false;

			if (CopyOnWrite())
				_map->emplace(key, value);
			else
				_map->emplace_hint(it, key, value);

			return true;
		}

		template < typename MapType__ >
		bool DoAdd(const KeyType& key, const ValueType& value, long)
		{
			CopyOnWrite();
			return _map->emplace(key, value).second;
		}

		template < typename MapType__ >
		auto DoSet(const KeyType& key, const ValueType& value, int) -> decltype(std::declval<MapType__>().lower_bound(key), void())
		{
			CopyOnWrite();
			const auto it = _map->lower_bound(key);
			if (it != _map->end() && !typename MapType__::key_compare()(key, it->first))
				it->second = value;
			else
				_map->emplace_hint(it, key, value);
		}

		template < typename MapType__ >
		void DoSet(const KeyType& key, const ValueType& value, long)
		{
			CopyOnWrite();
			const auto it = _map->find(key);
			if (it != _map->end())
				it->second = value;
			else
				_map->emplace(key, value);
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

		bool CopyOnWrite()
		{
			if (_mapHolder.expired())
				return false;

			CopyMap(_map);
			return true;
		}

		static shared_ptr<IEnumerator<PairType>> WrapMapEnumerator(const shared_ptr<IEnumerator<typename MapType::value_type>>& mapEnumerator)
		{ return WrapEnumerator(mapEnumerator); }
	};

	/** @} */

}

#endif
