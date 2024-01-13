#ifndef STINGRAYKIT_COLLECTION_GENERICLIST_H
#define STINGRAYKIT_COLLECTION_GENERICLIST_H

// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/IList.h>
#include <stingraykit/function/function.h>

#include <algorithm>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename VectorType_ >
	class GenericList : public virtual IList<typename VectorType_::value_type>
	{
	public:
		using ValueType = typename VectorType_::value_type;

	private:
		using VectorType = VectorType_;
		STINGRAYKIT_DECLARE_PTR(VectorType);

		struct Holder
		{
			const VectorTypePtr		Items;

			explicit Holder(const VectorTypePtr& items) : Items(items) { }
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
		VectorTypePtr			_items;
		mutable HolderWeakPtr	_itemsHolder;

	public:
		GenericList()
			:	_items(make_shared_ptr<VectorType>())
		{ }

		explicit GenericList(const shared_ptr<IEnumerable<ValueType>>& enumerable)
			:	GenericList(STINGRAYKIT_REQUIRE_NOT_NULL(enumerable)->GetEnumerator())
		{ }

		explicit GenericList(const shared_ptr<IEnumerator<ValueType>>& enumerator)
			:	_items(make_shared_ptr<VectorType>())
		{
			STINGRAYKIT_CHECK(enumerator, NullArgumentException("enumerator"));
			Enumerable::ForEach(enumerator, Bind(&GenericList::Add, this, _1));
		}

		GenericList(const GenericList& other)
		{ CopyItems(other._items); }

		GenericList& operator = (const GenericList& other)
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
		{ return std::find(_items->begin(), _items->end(), value) != _items->end(); }

		optional<size_t> IndexOf(const ValueType& value) const override
		{
			const auto it = std::find(_items->begin(), _items->end(), value);
			return it == _items->end() ? null : make_optional_value(std::distance(_items->begin(), it));
		}

		ValueType Get(size_t index) const override
		{
			STINGRAYKIT_CHECK(index < _items->size(), IndexOutOfRangeException(index, _items->size()));
			return (*_items)[index];
		}

		bool TryGet(size_t index, ValueType& value) const override
		{
			if (index >= _items->size())
				return false;

			value = (*_items)[index];
			return true;
		}

		void Add(const ValueType& value) override
		{
			CopyOnWrite();
			_items->push_back(value);
		}

		void Set(size_t index, const ValueType& value) override
		{
			STINGRAYKIT_CHECK(index < _items->size(), IndexOutOfRangeException(index, _items->size()));
			CopyOnWrite();
			(*_items)[index] = value;
		}

		void Insert(size_t index, const ValueType& value) override
		{
			STINGRAYKIT_CHECK(index <= _items->size(), IndexOutOfRangeException(index, _items->size()));
			CopyOnWrite();
			_items->insert(std::next(_items->begin(), index), value);
		}

		void RemoveAt(size_t index) override
		{
			STINGRAYKIT_CHECK(index < _items->size(), IndexOutOfRangeException(index, _items->size()));
			CopyOnWrite();
			_items->erase(std::next(_items->begin(), index));
		}

		bool Remove(const ValueType& value) override
		{
			const auto it = std::find(_items->begin(), _items->end(), value);
			if (it == _items->end())
				return false;

			const size_t index = std::distance(_items->begin(), it);

			CopyOnWrite();
			_items->erase(std::next(_items->begin(), index));
			return true;
		}

		size_t RemoveAll(const function<bool (const ValueType&)>& pred) override
		{
			CopyOnWrite();
			const auto it = std::remove_if(_items->begin(), _items->end(), pred);
			const size_t ret = std::distance(it, _items->end());
			_items->erase(it, _items->end());
			return ret;
		}

		void Clear() override
		{
			if (_itemsHolder.expired())
				_items->clear();
			else
			{
				_items = make_shared_ptr<VectorType>();
				_itemsHolder.reset();
			}
		}

	private:
		void CopyItems(const VectorTypePtr& items)
		{
			_items = make_shared_ptr<VectorType>(*items);
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

	/** @} */

}

#endif
