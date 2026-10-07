#ifndef STINGRAYKIT_COLLECTION_GENERICCOLLECTION_H
#define STINGRAYKIT_COLLECTION_GENERICCOLLECTION_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableFromStlContainer.h>
#include <stingraykit/collection/ICollection.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename CollectionType_, typename ItemType_ = typename CollectionType_::value_type >
	class GenericCollection
		:	public virtual ICollection<ItemType_>,
			public virtual IReversableEnumerable<ItemType_>
	{
	public:
		using ItemType = ItemType_;

		using CollectionType = CollectionType_;
		STINGRAYKIT_DECLARE_PTR(CollectionType);

	private:
		struct Holder
		{
			const CollectionTypePtr		Items;

			explicit Holder(const CollectionTypePtr& items) : Items(items) { }
		};
		STINGRAYKIT_DECLARE_PTR(Holder);

		class ReverseEnumerable : public virtual IEnumerable<ItemType>
		{
		private:
			const HolderPtr				_holder;

		public:
			explicit ReverseEnumerable(const HolderPtr& holder) : _holder(holder) { }

			shared_ptr<IEnumerator<ItemType>> GetEnumerator() const override
			{ return EnumeratorFromStlIterators<ItemType>(_holder->Items->rbegin(), _holder->Items->rend(), _holder); }
		};

	protected:
		CollectionTypePtr				_items;

	private:
		mutable HolderWeakPtr			_itemsHolder;
		mutable AtomicFlag::Type		_itemsHolderLock = 0;

	public:
		GenericCollection()
			:	_items(make_shared_ptr<CollectionType>())
		{ }

		shared_ptr<IEnumerator<ItemType>> GetEnumerator() const override final
		{ return EnumeratorFromStlContainer<ItemType>(*_items, GetItemsHolder()); }

		shared_ptr<IEnumerable<ItemType>> Reverse() const override final
		{ return make_shared_ptr<ReverseEnumerable>(GetItemsHolder()); }

		size_t GetCount() const override final
		{ return _items->size(); }

		bool IsEmpty() const override final
		{ return _items->empty(); }

	protected:
		void DoClear()
		{
			if (IsItemsHolderExpired())
				_items->clear();
			else
			{
				_items = make_shared_ptr<CollectionType>();
				ResetItemsHolder();
			}
		}

		HolderPtr GetItemsHolder() const
		{
			if (const HolderPtr itemsHolder = TryGetItemsHolder())
				return itemsHolder;

			return TrySetItemsHolder(make_shared_ptr<Holder>(_items));
		}

		bool CopyOnWrite()
		{
			if (IsItemsHolderExpired())
				return false;

			CopyItems(_items);
			return true;
		}

	private:
		void CopyItems(const CollectionTypePtr& items)
		{
			_items = make_shared_ptr<CollectionType>(*items);
			ResetItemsHolder();
		}

		bool IsItemsHolderExpired() const
		{
			Spinlock l(_itemsHolderLock);
			return _itemsHolder.expired();
		}

		HolderPtr TryGetItemsHolder() const
		{
			Spinlock l(_itemsHolderLock);
			return _itemsHolder.lock();
		}

		HolderPtr TrySetItemsHolder(const HolderPtr& holder) const
		{
			HolderPtr anotherHolder;

			Spinlock l(_itemsHolderLock);
			anotherHolder = _itemsHolder.lock();
			if (anotherHolder)
				return anotherHolder;

			_itemsHolder = holder;
			return holder;
		}

		void ResetItemsHolder()
		{
			HolderWeakPtr itemsHolder;

			Spinlock l(_itemsHolderLock);
			itemsHolder.swap(_itemsHolder);
		}
	};

	/** @} */

}

#endif
