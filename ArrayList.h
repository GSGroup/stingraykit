#ifndef STINGRAY_TOOLKIT_ARRAYLIST_H
#define STINGRAY_TOOLKIT_ARRAYLIST_H


#include <algorithm>
#include <vector>

#include <stingray/toolkit/EnumeratorFromStlContainer.h>
#include <stingray/toolkit/IEnumerable.h>
#include <stingray/toolkit/IList.h>
#include <stingray/toolkit/StlEnumeratorAdapter.h>


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
		TOOLKIT_DECLARE_PTR(VectorType);

		struct Holder
		{
			VectorTypePtr		Items;
			Holder(const VectorTypePtr& items) : Items(items) { }
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
		VectorTypePtr			_items;
		mutable HolderWeakPtr	_itemsEnumeratorHolder;

	public:
		ArrayList()
			: _items(new VectorType)
		{ }

		ArrayList(shared_ptr<IEnumerator<T> > enumerator)
		{
			TOOLKIT_REQUIRE_NOT_NULL(enumerator);
			std::copy(Wrap(enumerator), WrapEnd(enumerator), std::back_inserter(_items));
		}

		ArrayList(shared_ptr<IEnumerable<T> > enumerable)
		{
			TOOLKIT_REQUIRE_NOT_NULL(enumerable);
			shared_ptr<IEnumerator<T> > enumerator(enumerable->GetEnumerator());
			std::copy(Wrap(enumerator), WrapEnd(enumerator), std::back_inserter(_items));
		}

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
			TOOLKIT_CHECK(index >= 0 && index < (int)_items->size(), IndexOutOfRangeException(index, _items->size()));
			return (*_items)[index];
		}

		virtual void Set(int index, const ValueType& value)
		{
			CopyOnWrite();
			TOOLKIT_CHECK(index >= 0 && index < (int)_items->size(), IndexOutOfRangeException(index, _items->size()));
			(*_items)[index] = value;
		}

		virtual int IndexOf(const ValueType& value) const
		{
			typename VectorType::const_iterator it = std::find(_items->begin(), _items->end(), value);
			return it == _items->end() ? -1 : (it - _items->begin());
		}

		virtual void RemoveAt(int index)
		{
			CopyOnWrite();
			TOOLKIT_CHECK(index >= 0 && index < (int)_items->size(), IndexOutOfRangeException(index, _items->size()));
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
		{ _items->push_back(value); }

		virtual void Insert(int index, const ValueType& value)
		{
			TOOLKIT_CHECK(index >= 0 && index <= (int)_items->size(), IndexOutOfRangeException(index, _items->size()));

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
