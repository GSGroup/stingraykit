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
	class MultiMapDictionary
		:	public virtual IMultiDictionary<KeyType_, ValueType_>,
			public GenericCollection<std::multimap<KeyType_, ValueType_, KeyCompareType_>, KeyValuePair<KeyType_, ValueType_>>
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

		using BaseType = GenericCollection<MapType, PairType>;

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

		bool ContainsKey(const KeyType& key) const override
		{ return BaseType::_items->find(key) != BaseType::_items->end(); }

		size_t CountKey(const KeyType& key) const override
		{ return BaseType::_items->count(key); }

		shared_ptr<IEnumerator<PairType>> Find(const KeyType& key) const override
		{
			const auto it = BaseType::_items->lower_bound(key);
			if (it == BaseType::_items->end() || KeyCompareType()(key, it->first))
				return MakeEmptyEnumerator();

			return EnumeratorFromStlIterators<PairType>(it, BaseType::_items->end(), BaseType::GetItemsHolder());
		}

		shared_ptr<IEnumerator<PairType>> ReverseFind(const KeyType& key) const override
		{
			using cri = typename MapType::const_reverse_iterator;

			const auto it = BaseType::_items->upper_bound(key);
			if (it == BaseType::_items->end() || KeyCompareType()(cri(it)->first, key))
				return MakeEmptyEnumerator();

			return EnumeratorFromStlIterators<PairType>(cri(it), BaseType::_items->crend(), BaseType::GetItemsHolder());
		}

		ValueType GetFirst(const KeyType& key) const override
		{
			const auto it = BaseType::_items->lower_bound(key);
			STINGRAYKIT_CHECK(it != BaseType::_items->end() && !KeyCompareType()(key, it->first), CreateKeyNotFoundException(key));
			return it->second;
		}

		bool TryGetFirst(const KeyType& key, ValueType& outValue) const override
		{
			const auto it = BaseType::_items->lower_bound(key);
			if (it == BaseType::_items->end() || KeyCompareType()(key, it->first))
				return false;

			outValue = it->second;
			return true;
		}

		shared_ptr<IEnumerator<ValueType>> GetAll(const KeyType& key) const override
		{
			const auto range = BaseType::_items->equal_range(key);
			if (range.first == range.second)
				return MakeEmptyEnumerator();

			return EnumeratorFromStlIterators(values_iterator(range.first), values_iterator(range.second), BaseType::GetItemsHolder());
		}

		void Add(const KeyType& key, const ValueType& value) override
		{
			BaseType::CopyOnWrite();
			BaseType::_items->emplace(key, value);
		}

		bool RemoveFirst(const KeyType& key, const optional<ValueType>& value = null) override
		{
			const auto range = BaseType::_items->equal_range(key);

			size_t offset = 0;
			for (auto it = range.first; it != range.second; ++it, ++offset)
			{
				if (value && !ValueCompareType()(*value, it->second))
					continue;

				if (BaseType::CopyOnWrite())
					BaseType::_items->erase(std::next(BaseType::_items->lower_bound(key), offset));
				else
					BaseType::_items->erase(it);

				return true;
			}

			return false;
		}

		size_t RemoveAll(const KeyType& key) override
		{
			const auto range = BaseType::_items->equal_range(key);
			if (range.first == range.second)
				return 0;

			if (BaseType::CopyOnWrite())
				return BaseType::_items->erase(key);

			size_t ret = 0;
			for (auto it = range.first; it != range.second; )
			{
				it = BaseType::_items->erase(it);
				++ret;
			}

			return ret;
		}

		size_t RemoveWhere(const function<bool (const KeyType&, const ValueType&)>& pred) override
		{
			BaseType::CopyOnWrite();
			size_t ret = 0;
			for (auto it = BaseType::_items->begin(); it != BaseType::_items->end(); )
			{
				if (pred(it->first, it->second))
				{
					it = BaseType::_items->erase(it);
					++ret;
				}
				else
					++it;
			}
			return ret;
		}

		void Clear() override
		{ BaseType::DoClear(); }
	};

	/** @} */

}

#endif
