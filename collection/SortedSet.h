#ifndef STINGRAY_TOOLKIT_COLLECTION_SORTEDSET_H
#define STINGRAY_TOOLKIT_COLLECTION_SORTEDSET_H

#include <algorithm>
#include <set>

#include <stingray/toolkit/collection/EnumeratorFromStlContainer.h>
#include <stingray/toolkit/collection/IEnumerable.h>
#include <stingray/toolkit/collection/ISet.h>
#include <stingray/toolkit/collection/StlEnumeratorAdapter.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename T , typename CompareType_ = std::less<T> >
	class SortedSet : public virtual ISet<T>
	{
	public:
		typedef typename ISet<T>::ValueType				ValueType;

	private:
		typedef std::set<ValueType, CompareType_>		SetType;
		TOOLKIT_DECLARE_PTR(SetType);

		struct Holder
		{
			SetTypePtr		Items;
			Holder(const SetTypePtr& items) : Items(items) { }
		};
		TOOLKIT_DECLARE_PTR(Holder);

		struct ReverseEnumerable : public virtual IEnumerable<ValueType>
		{
			HolderPtr		_holder;

			ReverseEnumerable(const HolderPtr& holder) : _holder(holder) { }

			virtual shared_ptr<IEnumerator<ValueType> > GetEnumerator() const
			{ return EnumeratorFromStlIterators(_holder->Items->rbegin(), _holder->Items->rend(), _holder); }
		};

	private:
		shared_ptr<Mutex>		_mutex;
		SetTypePtr				_items;
		mutable HolderWeakPtr	_itemsEnumeratorHolder;

	public:
		SortedSet()
			: _mutex(new Mutex()), _items(new SetType)
		{ }

		SortedSet(const SortedSet& other)
			: _mutex(new Mutex()), _items(make_shared<SetType>(*other._items))
		{ }

		SortedSet& operator = (const SortedSet& other)
		{
			_items = make_shared<SetType>(*other._items);
			_itemsEnumeratorHolder.reset();
			return *this;
		}

		SortedSet(shared_ptr<IEnumerator<T> > enumerator)
			: _mutex(new Mutex())
		{
			TOOLKIT_REQUIRE_NOT_NULL(enumerator);
			std::copy(Wrap(enumerator), WrapEnd(enumerator), std::inserter(_items));
		}

		SortedSet(shared_ptr<IEnumerable<T> > enumerable)
			: _mutex(new Mutex())
		{
			TOOLKIT_REQUIRE_NOT_NULL(enumerable);
			shared_ptr<IEnumerator<T> > enumerator(enumerable->GetEnumerator());
			std::copy(Wrap(enumerator), WrapEnd(enumerator), std::inserter(_items));
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

		virtual void Clear()
		{ CopyOnWrite(); _items->clear(); }

		virtual bool IsEmpty() const
		{ return _items->empty(); }

		virtual void Add(const ValueType& value)
		{ CopyOnWrite(); _items->insert(value); }

		virtual void Remove(const ValueType& value)
		{ CopyOnWrite(); _items->erase(value); }

		virtual bool Contains(const ValueType& value) const
		{ return _items->find(value) != _items->end(); }

		virtual bool TryRemove(const ValueType& value)
		{
			typename SetType::iterator it = _items->find(value);
			if (it == _items->end())
				return false;

			CopyOnWrite();
			_items->erase(it);
			return true;
		}

	protected:
		shared_ptr<Mutex> GetMutexPointer() const
		{ return _mutex; }

	private:
		void CopyOnWrite()
		{
			if (_itemsEnumeratorHolder.lock())
			{
				_items.reset(new SetType(*_items));
				_itemsEnumeratorHolder.reset();
			}
		}
	};

	/** @} */

}


#endif
