#ifndef STINGRAYKIT_COLLECTION_SORTEDMULTISET_H
#define STINGRAYKIT_COLLECTION_SORTEDMULTISET_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/EnumeratorFromStlContainer.h>
#include <stingraykit/collection/IEnumerable.h>
#include <stingraykit/collection/IMultiSet.h>

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
	public:
		typedef typename IMultiSet<T>::ValueType				ValueType;

	private:
		typedef std::multiset<ValueType, CompareType_>	SetType;
		STINGRAYKIT_DECLARE_PTR(SetType);

		struct Holder
		{
			SetTypePtr		Items;
			Holder(const SetTypePtr& items) : Items(items) { }
		};
		STINGRAYKIT_DECLARE_PTR(Holder);

		struct ReverseEnumerable : public virtual IEnumerable<ValueType>
		{
			HolderPtr		_holder;

			ReverseEnumerable(const HolderPtr& holder) : _holder(holder) { }

			virtual shared_ptr<IEnumerator<ValueType> > GetEnumerator() const
			{ return EnumeratorFromStlIterators(_holder->Items->rbegin(), _holder->Items->rend(), _holder); }
		};

	private:
		SetTypePtr				_items;
		mutable HolderWeakPtr	_itemsEnumeratorHolder;

	public:
		SortedMultiSet()
			:	_items(make_shared<SetType>())
		{ }

		SortedMultiSet(const SortedMultiSet& other)
			:	_items(make_shared<SetType>(*other._items))
		{ }

		SortedMultiSet(shared_ptr<IEnumerator<T> > enumerator)
			:	_items(make_shared<SetType>())
		{
			STINGRAYKIT_REQUIRE_NOT_NULL(enumerator);
			Enumerable::ForEach(enumerator, bind(&SortedMultiSet::Add, this, _1));
		}

		SortedMultiSet(shared_ptr<IEnumerable<T> > enumerable)
			:	_items(make_shared<SetType>())
		{
			STINGRAYKIT_REQUIRE_NOT_NULL(enumerable);
			Enumerable::ForEach(enumerable, bind(&SortedMultiSet::Add, this, _1));
		}

		SortedMultiSet& operator = (const SortedMultiSet& other)
		{
			_items = make_shared<SetType>(*other._items);
			_itemsEnumeratorHolder.reset();
			return *this;
		}

		virtual shared_ptr<IEnumerator<ValueType> > GetEnumerator() const
		{
			shared_ptr<Holder> vector_enumerator_holder = _itemsEnumeratorHolder.lock();
			if (!vector_enumerator_holder)
				_itemsEnumeratorHolder = (vector_enumerator_holder = make_shared<Holder>(_items));

			return EnumeratorFromStlContainer(*_items, vector_enumerator_holder);
		}

		virtual shared_ptr<IEnumerable<ValueType> > Reverse() const
		{
			shared_ptr<Holder> vector_enumerator_holder = _itemsEnumeratorHolder.lock();
			if (!vector_enumerator_holder)
				_itemsEnumeratorHolder = (vector_enumerator_holder = make_shared<Holder>(_items));

			return make_shared<ReverseEnumerable>(vector_enumerator_holder);
		}

		virtual size_t GetCount() const
		{ return _items->size(); }

		virtual void Clear()
		{ CopyOnWrite(); _items->clear(); }

		virtual bool IsEmpty() const
		{ return _items->empty(); }

		virtual void Add(const ValueType& value)
		{ CopyOnWrite(); _items->insert(value); }

		virtual void Remove(const ValueType& value)
		{ TryRemove(value); }

		virtual void RemoveAll(const ValueType& value)
		{ CopyOnWrite(); _items->erase(value); }

		virtual bool Contains(const ValueType& value) const
		{ return _items->find(value) != _items->end(); }

		virtual int Count(const ValueType& value) const
		{ return _items->count(value); }

		virtual bool TryRemove(const ValueType& value)
		{
			typename SetType::iterator it = _items->find(value);
			if (it == _items->end())
				return false;

			CopyOnWrite();
			_items->erase(_items->find(value));
			return true;
		}

	private:
		void CopyOnWrite()
		{
			if (_itemsEnumeratorHolder.lock())
			{
				_items = make_shared<SetType>(*_items);
				_itemsEnumeratorHolder.reset();
			}
		}
	};

	/** @} */

}

#endif
