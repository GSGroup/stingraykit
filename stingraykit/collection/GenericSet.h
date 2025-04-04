#ifndef STINGRAYKIT_COLLECTION_GENERICSET_H
#define STINGRAYKIT_COLLECTION_GENERICSET_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/ISet.h>
#include <stingraykit/function/function.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename SetType_ >
	class GenericSet : public virtual ISet<typename SetType_::value_type>
	{
		static_assert(comparers::IsRelationalComparer<typename SetType_::value_compare>::Value, "Expected Relational comparer");

	public:
		using ValueType = typename SetType_::value_type;

	private:
		using SetType = SetType_;
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
		GenericSet()
			:	_items(make_shared_ptr<SetType>())
		{ }

		explicit GenericSet(const shared_ptr<IEnumerable<ValueType>>& enumerable)
			:	GenericSet(STINGRAYKIT_REQUIRE_NOT_NULL(enumerable)->GetEnumerator())
		{ }

		explicit GenericSet(const shared_ptr<IEnumerator<ValueType>>& enumerator)
			:	_items(make_shared_ptr<SetType>())
		{
			STINGRAYKIT_CHECK(enumerator, NullArgumentException("enumerator"));
			Enumerable::ForEach(enumerator, Bind(&GenericSet::Add, this, _1));
		}

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
			const auto it = _items->find(value);
			if (it == _items->end())
				return MakeEmptyEnumerator();

			return EnumeratorFromStlIterators(it, _items->end(), GetItemsHolder());
		}

		shared_ptr<IEnumerator<ValueType>> ReverseFind(const ValueType& value) const override
		{
			auto it = _items->find(value);
			if (it == _items->end())
				return MakeEmptyEnumerator();

			return EnumeratorFromStlIterators(typename SetType::const_reverse_iterator(++it), _items->crend(), GetItemsHolder());
		}

		bool Add(const ValueType& value) override
		{ return DoAdd<SetType>(value, 0); }

		bool Remove(const ValueType& value) override
		{
			const auto it = _items->find(value);
			if (it == _items->end())
				return false;

			if (CopyOnWrite())
				_items->erase(value);
			else
				_items->erase(it);

			return true;
		}

		size_t RemoveWhere(const function<bool (const ValueType&)>& pred) override
		{
			CopyOnWrite();
			size_t ret = 0;
			for (auto it = _items->begin(); it != _items->end(); )
			{
				if (pred(*it))
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
				_items = make_shared_ptr<SetType>();
				_itemsHolder.reset();
			}
		}

	private:
		template < typename SetType__ >
		auto DoAdd(const ValueType& value, int) -> decltype(std::declval<SetType__>().lower_bound(value), bool())
		{
			const auto it = _items->lower_bound(value);
			if (it != _items->end() && !typename SetType__::value_compare()(value, *it))
				return false;

			if (CopyOnWrite())
				_items->insert(value);
			else
				_items->insert(it, value);

			return true;
		}

		template < typename SetType__ >
		bool DoAdd(const ValueType& value, long)
		{
			CopyOnWrite();
			return _items->insert(value).second;
		}

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
