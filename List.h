#ifndef __GS_DVRLIB_TOOLKIT_LIST_H__
#define __GS_DVRLIB_TOOLKIT_LIST_H__


#include <list>
#include <algorithm>

#include <stingray/toolkit/EnumeratorFromStlContainer.h>
#include <stingray/toolkit/StlEnumeratorAdapter.h>
#include <stingray/toolkit/IList.h>


namespace stingray
{

	template < typename T >
	class List : public virtual IList<T>
	{
		typedef typename GetConstReferenceType<T>::ValueT	ConstTRef;
		typedef std::list<T>								Items;
		TOOLKIT_NONCOPYABLE(List);
	private:
		Items		_items;

	public:
		List() { }

		List(shared_ptr<IEnumerator<T> > enumerator)
		{
			TOOLKIT_REQUIRE_NOT_NULL(enumerator);
			std::copy(Wrap(enumerator), WrapEnd(enumerator), std::back_inserter(_items));
		}

		List(shared_ptr<IEnumerable<T> > enumerable)
		{
			TOOLKIT_REQUIRE_NOT_NULL(enumerable);
			shared_ptr<IEnumerator<T> > enumerator(enumerable->GetEnumerator());
			std::copy(Wrap(enumerator), WrapEnd(enumerator), std::back_inserter(_items));
		}

		virtual shared_ptr<IEnumerator<T> > GetEnumerator()
		{ return EnumeratorFromStlContainer(_items); }

		virtual int GetCount() const
		{ return _items.size(); }

		virtual void Add(ConstTRef obj)
		{ _items.push_back(obj); }

		virtual void Insert(size_t index, ConstTRef obj)
		{
			if (!(index < _items.size()))
				TOOLKIT_THROW(IndexOutOfRangeException());

			typename Items::iterator it = _items.begin();
			std::advance(it, index);
			_items.insert(it, obj);
		}

		virtual T Get(size_t index) const 
		{ 
			if (index >= _items.size())
				TOOLKIT_THROW(IndexOutOfRangeException());
			typename Items::const_iterator it = _items.begin();
			std::advance(it, index); 
			return *it;
		}

		virtual void Remove(ConstTRef obj)
		{
			for (typename Items::iterator i = _items.begin(); i != _items.end(); ++i)
				if ((*i) == obj)
				{
					_items.erase(i); // TODO: reimplement?
					return;
				}
		}

		virtual void Remove(size_t index)
		{
			if (!(index < _items.size()))
				TOOLKIT_THROW(IndexOutOfRangeException());

			typename Items::iterator it = _items.begin();
			std::advance(it, index);
			_items.erase(it);
		}
	};


}


#endif
