#ifndef STINGRAYKIT_COLLECTION_MULTIMAPDICTIONARY_H
#define STINGRAYKIT_COLLECTION_MULTIMAPDICTIONARY_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/ForEach.h>
#include <stingraykit/collection/IMultiDictionary.h>
#include <stingraykit/collection/KeyExceptionCreator.h>
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
		static_assert(comparers::IsRelationalComparer<KeyCompareType_>::Value, "Expected Relational comparer");
		static_assert(comparers::IsEqualsComparer<ValueCompareType_>::Value, "Expected Equals comparer");

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
			const MapTypePtr		Items;

			explicit Holder(const MapTypePtr& items) : Items(items) { }
		};
		STINGRAYKIT_DECLARE_PTR(Holder);

		class ReverseEnumerable : public virtual IEnumerable<PairType>
		{
		private:
			const HolderPtr			_holder;

		public:
			explicit ReverseEnumerable(const HolderPtr& holder) : _holder(holder) { }

			shared_ptr<IEnumerator<PairType>> GetEnumerator() const override
			{ return EnumeratorFromStlIterators<PairType>(_holder->Items->rbegin(), _holder->Items->rend(), _holder); }
		};

	private:
		MapTypePtr				_items;
		mutable HolderWeakPtr	_itemsHolder;

	public:
		MultiMapDictionary()
			:	_items(make_shared_ptr<MapType>())
		{ }

		explicit MultiMapDictionary(const shared_ptr<IEnumerable<PairType>>& enumerable)
			:	MultiMapDictionary(STINGRAYKIT_REQUIRE_NOT_NULL(enumerable)->GetEnumerator())
		{ }

		explicit MultiMapDictionary(const shared_ptr<IEnumerator<PairType>>& enumerator)
			:	_items(make_shared_ptr<MapType>())
		{
			STINGRAYKIT_CHECK(enumerator, NullArgumentException("enumerator"));
			FOR_EACH(const PairType pair IN enumerator)
				Add(pair.Key, pair.Value);
		}

		shared_ptr<IEnumerator<PairType>> GetEnumerator() const override
		{ return EnumeratorFromStlContainer<PairType>(*_items, GetItemsHolder()); }

		shared_ptr<IEnumerable<PairType>> Reverse() const override
		{ return make_shared_ptr<ReverseEnumerable>(GetItemsHolder()); }

		size_t GetCount() const override
		{ return _items->size(); }

		bool IsEmpty() const override
		{ return _items->empty(); }

		bool ContainsKey(const KeyType& key) const override
		{ return _items->find(key) != _items->end(); }

		size_t CountKey(const KeyType& key) const override
		{ return _items->count(key); }

		shared_ptr<IEnumerator<PairType>> Find(const KeyType& key) const override
		{
			const auto it = _items->lower_bound(key);
			if (it == _items->end() || KeyCompareType()(key, it->first))
				return MakeEmptyEnumerator();

			return EnumeratorFromStlIterators<PairType>(it, _items->end(), GetItemsHolder());
		}

		shared_ptr<IEnumerator<PairType>> ReverseFind(const KeyType& key) const override
		{
			using cri = typename MapType::const_reverse_iterator;

			const auto it = _items->upper_bound(key);
			if (it == _items->end() || KeyCompareType()(cri(it)->first, key))
				return MakeEmptyEnumerator();

			return EnumeratorFromStlIterators<PairType>(cri(it), _items->crend(), GetItemsHolder());
		}

		ValueType GetFirst(const KeyType& key) const override
		{
			const auto it = _items->lower_bound(key);
			STINGRAYKIT_CHECK(it != _items->end() && !KeyCompareType()(key, it->first), CreateKeyNotFoundException(key));
			return it->second;
		}

		bool TryGetFirst(const KeyType& key, ValueType& outValue) const override
		{
			const auto it = _items->lower_bound(key);
			if (it == _items->end() || KeyCompareType()(key, it->first))
				return false;

			outValue = it->second;
			return true;
		}

		shared_ptr<IEnumerator<ValueType>> GetAll(const KeyType& key) const override
		{
			const auto range = _items->equal_range(key);
			if (range.first == range.second)
				return MakeEmptyEnumerator();

			return EnumeratorFromStlIterators(values_iterator(range.first), values_iterator(range.second), GetItemsHolder());
		}

		void Add(const KeyType& key, const ValueType& value) override
		{
			CopyOnWrite();
			_items->emplace(key, value);
		}

		bool RemoveFirst(const KeyType& key, const optional<ValueType>& value = null) override
		{
			const auto range = _items->equal_range(key);

			size_t offset = 0;
			for (auto it = range.first; it != range.second; ++it, ++offset)
			{
				if (value && !ValueCompareType()(*value, it->second))
					continue;

				if (CopyOnWrite())
					_items->erase(std::next(_items->lower_bound(key), offset));
				else
					_items->erase(it);

				return true;
			}

			return false;
		}

		size_t RemoveAll(const KeyType& key) override
		{
			const auto range = _items->equal_range(key);
			if (range.first == range.second)
				return 0;

			if (CopyOnWrite())
				return _items->erase(key);

			size_t ret = 0;
			for (auto it = range.first; it != range.second; )
			{
				it = _items->erase(it);
				++ret;
			}

			return ret;
		}

		size_t RemoveWhere(const function<bool (const KeyType&, const ValueType&)>& pred) override
		{
			CopyOnWrite();
			size_t ret = 0;
			for (auto it = _items->begin(); it != _items->end(); )
			{
				if (pred(it->first, it->second))
				{
					it = _items->erase(it);
					++ret;
				}
				else
					++it;
			}
			return ret;
		}

		void Clear() override
		{
			if (_itemsHolder.expired())
				_items->clear();
			else
			{
				_items = make_shared_ptr<MapType>();
				_itemsHolder.reset();
			}
		}

	private:
		void CopyItems(const MapTypePtr& items)
		{
			_items = make_shared_ptr<MapType>(*items);
			_itemsHolder.reset();
		}

		HolderPtr GetItemsHolder() const
		{
			HolderPtr itemsHolder = _itemsHolder.lock();

			if (!itemsHolder)
				_itemsHolder = (itemsHolder = make_shared_ptr<Holder>(_items));

			return itemsHolder;
		}

		bool CopyOnWrite()
		{
			if (_itemsHolder.expired())
				return false;

			CopyItems(_items);
			return true;
		}
	};

	/** @} */

}

#endif
