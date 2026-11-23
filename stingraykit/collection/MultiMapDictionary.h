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
#include <stingraykit/collection/GenericCollection.h>
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

		using ImplType = GenericCollection<MapType, PairType>;

	private:
		ImplType			_impl;

	public:
		MultiMapDictionary()
		{ }

		explicit MultiMapDictionary(const shared_ptr<IEnumerable<PairType>>& enumerable)
			:	MultiMapDictionary(STINGRAYKIT_REQUIRE_NOT_NULL(enumerable)->GetEnumerator())
		{ }

		explicit MultiMapDictionary(const shared_ptr<IEnumerator<PairType>>& enumerator)
		{
			STINGRAYKIT_CHECK(enumerator, NullArgumentException("enumerator"));
			FOR_EACH(const PairType pair IN enumerator)
				Add(pair.Key, pair.Value);
		}

		shared_ptr<IEnumerator<PairType>> GetEnumerator() const override
		{ return _impl.GetEnumerator(); }

		shared_ptr<IEnumerable<PairType>> Reverse() const override
		{ return _impl.Reverse(); }

		size_t GetCount() const override
		{ return _impl.GetCount(); }

		bool IsEmpty() const override
		{ return _impl.IsEmpty(); }

		bool ContainsKey(const KeyType& key) const override
		{
			const MapType& items = _impl.GetItems();
			return items.find(key) != items.end();
		}

		size_t CountKey(const KeyType& key) const override
		{ return _impl.GetItems().count(key); }

		shared_ptr<IEnumerator<PairType>> Find(const KeyType& key) const override
		{
			const MapType& items = _impl.GetItems();

			const auto it = items.lower_bound(key);
			if (it == items.end() || KeyCompareType()(key, it->first))
				return MakeEmptyEnumerator();

			return EnumeratorFromStlIterators<PairType>(it, items.end(), _impl.GetItemsHolder());
		}

		shared_ptr<IEnumerator<PairType>> ReverseFind(const KeyType& key) const override
		{
			using cri = typename MapType::const_reverse_iterator;

			const MapType& items = _impl.GetItems();

			const auto it = items.upper_bound(key);
			if (it == items.end() || KeyCompareType()(cri(it)->first, key))
				return MakeEmptyEnumerator();

			return EnumeratorFromStlIterators<PairType>(cri(it), items.crend(), _impl.GetItemsHolder());
		}

		ValueType GetFirst(const KeyType& key) const override
		{
			const MapType& items = _impl.GetItems();

			const auto it = items.lower_bound(key);
			STINGRAYKIT_CHECK(it != items.end() && !KeyCompareType()(key, it->first), CreateKeyNotFoundException(key));

			return it->second;
		}

		bool TryGetFirst(const KeyType& key, ValueType& outValue) const override
		{
			const MapType& items = _impl.GetItems();

			const auto it = items.lower_bound(key);
			if (it == items.end() || KeyCompareType()(key, it->first))
				return false;

			outValue = it->second;
			return true;
		}

		shared_ptr<IEnumerator<ValueType>> GetAll(const KeyType& key) const override
		{
			const auto range = _impl.GetItems().equal_range(key);
			if (range.first == range.second)
				return MakeEmptyEnumerator();

			return EnumeratorFromStlIterators(values_iterator(range.first), values_iterator(range.second), _impl.GetItemsHolder());
		}

		void Add(const KeyType& key, const ValueType& value) override
		{
			_impl.CopyOnWrite();
			_impl.GetItems().emplace(key, value);
		}

		bool RemoveFirst(const KeyType& key, const optional<ValueType>& value = null) override
		{
			MapType& items = _impl.GetItems();

			const auto range = items.equal_range(key);

			size_t offset = 0;
			for (auto it = range.first; it != range.second; ++it, ++offset)
			{
				if (value && !ValueCompareType()(*value, it->second))
					continue;

				if (_impl.CopyOnWrite())
					_impl.GetItems().erase(std::next(_impl.GetItems().lower_bound(key), offset));
				else
					items.erase(it);

				return true;
			}

			return false;
		}

		size_t RemoveAll(const KeyType& key) override
		{
			MapType& items = _impl.GetItems();

			const auto range = items.equal_range(key);
			if (range.first == range.second)
				return 0;

			if (_impl.CopyOnWrite())
				return _impl.GetItems().erase(key);

			size_t ret = 0;
			for (auto it = range.first; it != range.second; )
			{
				it = items.erase(it);
				++ret;
			}

			return ret;
		}

		size_t RemoveWhere(const function<bool (const KeyType&, const ValueType&)>& pred) override
		{
			_impl.CopyOnWrite();

			MapType& items = _impl.GetItems();

			size_t ret = 0;
			for (auto it = items.begin(); it != items.end(); )
			{
				if (pred(it->first, it->second))
				{
					it = items.erase(it);
					++ret;
				}
				else
					++it;
			}

			return ret;
		}

		void Clear() override
		{ _impl.Clear(); }
	};

	/** @} */

}

#endif
