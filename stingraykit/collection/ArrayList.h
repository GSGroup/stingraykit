#ifndef STINGRAYKIT_COLLECTION_ARRAYLIST_H
#define STINGRAYKIT_COLLECTION_ARRAYLIST_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <algorithm>
#include <vector>

#include <stingraykit/collection/EnumeratorFromStlContainer.h>
#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/IEnumerable.h>
#include <stingraykit/collection/IList.h>
#include <stingraykit/function/bind.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename T >
	class ArrayList : public virtual IList<T>
	{
	public:
		typedef typename IList<T>::ValueType				ValueType;

	private:
		typedef std::vector<ValueType>						VectorType;
		STINGRAYKIT_DECLARE_PTR(VectorType);

		struct Holder
		{
			VectorTypePtr		Items;
			Holder(const VectorTypePtr& items) : Items(items) { }
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
		VectorTypePtr			_items;
		mutable HolderWeakPtr	_itemsEnumeratorHolder;

	public:
		ArrayList()
			: _items(new VectorType)
		{ }

		ArrayList(shared_ptr<IEnumerator<T> > enumerator) : _items(new VectorType)
		{ Enumerable::ForEach(enumerator, bind(&ArrayList::Add, this, _1)); }

		ArrayList(shared_ptr<IEnumerable<T> > enumerable) : _items(new VectorType)
		{ Enumerable::ForEach(enumerable, bind(&ArrayList::Add, this, _1)); }

		ArrayList(const ArrayList& other) : _items(new VectorType(*other._items))
		{ }

		ArrayList& operator = (const ArrayList& other)
		{
			_items = make_shared<VectorType>(*other._items);
			_itemsEnumeratorHolder.reset();
			return *this;
		}

		virtual ValueType Get(int index) const
		{
			STINGRAYKIT_CHECK(index >= 0 && index < (int)_items->size(), IndexOutOfRangeException(index, _items->size()));
			return (*_items)[index];
		}

		virtual void Set(int index, const ValueType& value)
		{
			STINGRAYKIT_CHECK(index >= 0 && index < (int)_items->size(), IndexOutOfRangeException(index, _items->size()));
			CopyOnWrite();
			(*_items)[index] = value;
		}

		virtual int IndexOf(const ValueType& value) const
		{
			typename VectorType::const_iterator it = std::find(_items->begin(), _items->end(), value);
			return it == _items->end() ? -1 : (it - _items->begin());
		}

		virtual void RemoveAt(int index)
		{
			STINGRAYKIT_CHECK(index >= 0 && index < (int)_items->size(), IndexOutOfRangeException(index, _items->size()));
			CopyOnWrite();
			_items->erase(_items->begin() + index);
		}

		virtual void Clear()
		{
			CopyOnWrite();
			_items->clear();
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

		virtual int GetCount() const
		{ return _items->size(); }

		virtual bool IsEmpty() const
		{ return _items->empty(); }

		virtual void Add(const ValueType& value)
		{
			CopyOnWrite();
			_items->push_back(value);
		}

		virtual void Insert(int index, const ValueType& value)
		{
			STINGRAYKIT_CHECK(index >= 0 && index <= (int)_items->size(), IndexOutOfRangeException(index, _items->size()));
			CopyOnWrite();

			typename VectorType::iterator it = _items->begin();
			std::advance(it, index);
			_items->insert(it, value);
		}

	private:
		void CopyOnWrite()
		{
			if (_itemsEnumeratorHolder.lock())
			{
				_items.reset(new VectorType(*_items));
				_itemsEnumeratorHolder.reset();
			}
		}
	};

	/** @} */

}


#endif
