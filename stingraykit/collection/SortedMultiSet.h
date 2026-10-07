#ifndef STINGRAYKIT_COLLECTION_SORTEDMULTISET_H
#define STINGRAYKIT_COLLECTION_SORTEDMULTISET_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/GenericCollection.h>
#include <stingraykit/collection/IMultiSet.h>
#include <stingraykit/function/function.h>

#include <set>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename T , typename CompareType_ = comparers::Less >
	class SortedMultiSet
		:	public virtual IMultiSet<T>,
			public GenericCollection<std::multiset<T, CompareType_>>
	{
		static_assert(comparers::IsRelationalComparer<CompareType_>::Value, "Expected Relational comparer");

	public:
		using ValueType = typename IMultiSet<T>::ValueType;

	private:
		using SetType = std::multiset<ValueType, CompareType_>;

		using BaseType = GenericCollection<SetType>;

	public:
		SortedMultiSet()
		{ }

		explicit SortedMultiSet(const shared_ptr<IEnumerable<T>>& enumerable)
			:	SortedMultiSet(STINGRAYKIT_REQUIRE_NOT_NULL(enumerable)->GetEnumerator())
		{ }

		explicit SortedMultiSet(const shared_ptr<IEnumerator<T>>& enumerator)
		{
			STINGRAYKIT_CHECK(enumerator, NullArgumentException("enumerator"));
			Enumerable::ForEach(enumerator, Bind(&SortedMultiSet::Add, this, _1));
		}

		bool Contains(const ValueType& value) const override
		{ return BaseType::_items->find(value) != BaseType::_items->end(); }

		size_t Count(const ValueType& value) const override
		{ return BaseType::_items->count(value); }

		shared_ptr<IEnumerator<ValueType>> Find(const ValueType& value) const override
		{
			const auto it = BaseType::_items->lower_bound(value);
			if (it == BaseType::_items->end() || CompareType_()(value, *it))
				return MakeEmptyEnumerator();

			return EnumeratorFromStlIterators(it, BaseType::_items->end(), BaseType::GetItemsHolder());
		}

		shared_ptr<IEnumerator<ValueType>> ReverseFind(const ValueType& value) const override
		{
			using cri = typename SetType::const_reverse_iterator;

			const auto it = BaseType::_items->upper_bound(value);
			if (it == BaseType::_items->end() || CompareType_()(*cri(it), value))
				return MakeEmptyEnumerator();

			return EnumeratorFromStlIterators(cri(it), BaseType::_items->crend(), BaseType::GetItemsHolder());
		}

		void Add(const ValueType& value) override
		{
			BaseType::CopyOnWrite();
			BaseType::_items->insert(value);
		}

		bool RemoveFirst(const ValueType& value) override
		{
			const auto it = BaseType::_items->lower_bound(value);
			if (it == BaseType::_items->end() || CompareType_()(value, *it))
				return false;

			if (BaseType::CopyOnWrite())
				BaseType::_items->erase(BaseType::_items->lower_bound(value));
			else
				BaseType::_items->erase(it);

			return true;
		}

		size_t RemoveAll(const ValueType& value) override
		{
			const auto range = BaseType::_items->equal_range(value);
			if (range.first == range.second)
				return 0;

			if (BaseType::CopyOnWrite())
				return BaseType::_items->erase(value);

			size_t ret = 0;
			for (auto it = range.first; it != range.second; )
			{
				it = BaseType::_items->erase(it);
				++ret;
			}

			return ret;
		}

		size_t RemoveWhere(const function<bool (const ValueType&)>& pred) override
		{
			BaseType::CopyOnWrite();
			size_t ret = 0;
			for (auto it = BaseType::_items->begin(); it != BaseType::_items->end(); )
			{
				if (pred(*it))
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
