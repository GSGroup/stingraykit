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
	class SortedMultiSet : public virtual IMultiSet<T>
	{
		static_assert(comparers::IsRelationalComparer<CompareType_>::Value, "Expected Relational comparer");

	public:
		using ValueType = typename IMultiSet<T>::ValueType;

	private:
		using SetType = std::multiset<ValueType, CompareType_>;

		using ImplType = GenericCollection<SetType>;

	private:
		ImplType			_impl;

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

		shared_ptr<IEnumerator<ValueType>> GetEnumerator() const override
		{ return _impl.GetEnumerator(); }

		shared_ptr<IEnumerable<ValueType>> Reverse() const override
		{ return _impl.Reverse(); }

		size_t GetCount() const override
		{ return _impl.GetCount(); }

		bool IsEmpty() const override
		{ return _impl.IsEmpty(); }

		bool Contains(const ValueType& value) const override
		{
			const SetType& items = _impl.GetItems();
			return items.find(value) != items.end();
		}

		size_t Count(const ValueType& value) const override
		{ return _impl.GetItems().count(value); }

		shared_ptr<IEnumerator<ValueType>> Find(const ValueType& value) const override
		{
			const SetType& items = _impl.GetItems();

			const auto it = items.lower_bound(value);
			if (it == items.end() || CompareType_()(value, *it))
				return MakeEmptyEnumerator();

			return EnumeratorFromStlIterators(it, items.end(), _impl.GetItemsHolder());
		}

		shared_ptr<IEnumerator<ValueType>> ReverseFind(const ValueType& value) const override
		{
			using cri = typename SetType::const_reverse_iterator;

			const SetType& items = _impl.GetItems();

			const auto it = items.upper_bound(value);
			if (it == items.end() || CompareType_()(*cri(it), value))
				return MakeEmptyEnumerator();

			return EnumeratorFromStlIterators(cri(it), items.crend(), _impl.GetItemsHolder());
		}

		void Add(const ValueType& value) override
		{
			_impl.CopyOnWrite();
			_impl.GetItems().insert(value);
		}

		bool RemoveFirst(const ValueType& value) override
		{
			SetType& items = _impl.GetItems();

			const auto it = items.lower_bound(value);
			if (it == items.end() || CompareType_()(value, *it))
				return false;

			if (_impl.CopyOnWrite())
			{
				SetType& newItems = _impl.GetItems();
				newItems.erase(newItems.lower_bound(value));
			}
			else
				items.erase(it);

			return true;
		}

		size_t RemoveAll(const ValueType& value) override
		{
			SetType& items = _impl.GetItems();

			const auto range = items.equal_range(value);
			if (range.first == range.second)
				return 0;

			if (_impl.CopyOnWrite())
				return _impl.GetItems().erase(value);

			size_t ret = 0;
			for (auto it = range.first; it != range.second; )
			{
				it = items.erase(it);
				++ret;
			}

			return ret;
		}

		size_t RemoveWhere(const function<bool (const ValueType&)>& pred) override
		{
			_impl.CopyOnWrite();

			SetType& items = _impl.GetItems();

			size_t ret = 0;
			for (auto it = items.begin(); it != items.end(); )
			{
				if (pred(*it))
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
