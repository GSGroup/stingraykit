#ifndef STINGRAYKIT_COLLECTION_SORTEDSET_H
#define STINGRAYKIT_COLLECTION_SORTEDSET_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/ISet.h>
#include <stingraykit/collection/flat_set.h>
#include <stingraykit/function/function.h>

#include <set>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template <
			typename T,
			typename CompareType_ = comparers::Less,
			template <class, class, class> class SetType_ = std::set,
			typename AllocatorType_ = std::allocator<T>
			>
	class SortedSet : public virtual ISet<T>
	{
	public:
		using ValueType = typename ISet<T>::ValueType;

	private:
		using SetType = SetType_<ValueType, CompareType_, AllocatorType_>;
		STINGRAYKIT_DECLARE_PTR(SetType);

		struct Holder
		{
			const SetTypePtr		Items;

			explicit Holder(const SetTypePtr& items) : Items(items) { }
		};
		STINGRAYKIT_DECLARE_PTR(Holder);

		class ReverseEnumerable : public virtual IEnumerable<ValueType>
		{
		private:
			const HolderPtr			_holder;

		public:
			explicit ReverseEnumerable(const HolderPtr& holder) : _holder(holder) { }

			shared_ptr<IEnumerator<ValueType>> GetEnumerator() const override
			{ return EnumeratorFromStlIterators(_holder->Items->rbegin(), _holder->Items->rend(), _holder); }
		};

	private:
		SetTypePtr				_items;
		mutable HolderWeakPtr	_itemsHolder;

	public:
		SortedSet()
			:	_items(make_shared_ptr<SetType>())
		{ }

		SortedSet(const SortedSet& other)
		{ CopyItems(other._items); }

		explicit SortedSet(const shared_ptr<IEnumerable<ValueType>>& enumerable)
			:	SortedSet(STINGRAYKIT_REQUIRE_NOT_NULL(enumerable)->GetEnumerator())
		{ }

		explicit SortedSet(const shared_ptr<IEnumerator<ValueType>>& enumerator)
			:	_items(make_shared_ptr<SetType>())
		{
			STINGRAYKIT_CHECK(enumerator, NullArgumentException("enumerator"));
			Enumerable::ForEach(enumerator, Bind(&SortedSet::Add, this, _1));
		}

		SortedSet& operator = (const SortedSet& other)
		{ CopyItems(other._items); return *this; }

		shared_ptr<IEnumerator<ValueType>> GetEnumerator() const override
		{ return EnumeratorFromStlContainer(*_items, GetItemsHolder()); }

		shared_ptr<IEnumerable<ValueType>> Reverse() const override
		{ return make_shared_ptr<ReverseEnumerable>(GetItemsHolder()); }

		size_t GetCount() const override
		{ return _items->size(); }

		bool IsEmpty() const override
		{ return _items->empty(); }

		bool Contains(const ValueType& value) const override
		{ return _items->find(value) != _items->end(); }

		shared_ptr<IEnumerator<ValueType>> Find(const ValueType& value) const override
		{
			typename SetType::const_iterator it = _items->find(value);
			if (it == _items->end())
				return MakeEmptyEnumerator();

			return EnumeratorFromStlIterators(it, _items->end(), GetItemsHolder());
		}

		shared_ptr<IEnumerator<ValueType>> ReverseFind(const ValueType& value) const override
		{
			using cri = typename SetType::const_reverse_iterator;

			typename SetType::const_iterator it = _items->find(value);
			if (it == _items->end())
				return MakeEmptyEnumerator();

			return EnumeratorFromStlIterators(cri(++it), _items->rend(), GetItemsHolder());
		}

		void Add(const ValueType& value) override
		{
			CopyOnWrite();
			_items->insert(value);
		}

		void Remove(const ValueType& value) override
		{
			CopyOnWrite();
			_items->erase(value);
		}

		bool TryRemove(const ValueType& value) override
		{
			typename SetType::iterator it = _items->find(value);
			if (it == _items->end())
				return false;

			CopyOnWrite();
			_items->erase(value);
			return true;
		}

		size_t RemoveWhere(const function<bool (const ValueType&)>& pred) override
		{
			CopyOnWrite();
			size_t ret = 0;
			for (typename SetType::iterator it = _items->begin(); it != _items->end(); )
			{
				const typename SetType::iterator cur = it++;
				if (!pred(*cur))
					continue;

				_items->erase(cur);
				++ret;
			}
			return ret;
		}

		void Clear() override
		{
			if (_itemsHolder.expired())
				_items->clear();
			else
			{
				_items = make_shared_ptr<SetType>();
				_itemsHolder.reset();
			}
		}

	private:
		void CopyItems(const SetTypePtr& items)
		{
			_items = make_shared_ptr<SetType>(*items);
			_itemsHolder.reset();
		}

		HolderPtr GetItemsHolder() const
		{
			HolderPtr itemsHolder = _itemsHolder.lock();

			if (!itemsHolder)
				_itemsHolder = (itemsHolder = make_shared_ptr<Holder>(_items));

			return itemsHolder;
		}

		void CopyOnWrite()
		{
			if (!_itemsHolder.expired())
				CopyItems(_items);
		}
	};


	template <
			typename T,
			typename CompareType = comparers::Less,
			typename AllocatorType = typename flat_set<T, CompareType>::allocator_type
			>
	struct FlatSortedSet
	{ using Type = SortedSet<T, CompareType, flat_set, AllocatorType>; };

	/** @} */

}


#endif
