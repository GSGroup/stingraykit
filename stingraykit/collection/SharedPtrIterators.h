#ifndef STINGRAYKIT_COLLECTION_SHAREDPTRITERATORS_H
#define STINGRAYKIT_COLLECTION_SHAREDPTRITERATORS_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/shared_ptr.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

template<typename Container>
struct shared_copy_back_insert_iterator : public std::iterator<std::output_iterator_tag, void, void, void, void>
{
	typedef typename Container::value_type SharedItemT;
	typedef typename GetSharedPtrParam<SharedItemT>::ValueT RawItemT;
private:
	Container& _container;
public:
	explicit shared_copy_back_insert_iterator(Container& container) : _container(container) { }
	shared_copy_back_insert_iterator& operator =(const RawItemT& value)
	{
		_container.push_back(SharedItemT(new RawItemT(value)));
		return *this;
	}
	shared_copy_back_insert_iterator& operator *()		{ return *this; }
	shared_copy_back_insert_iterator& operator ++()		{ return *this; }
	shared_copy_back_insert_iterator  operator ++(int)	{ return *this; }
};

template<typename Container>
inline shared_copy_back_insert_iterator<Container> shared_copy_back_inserter(Container& container)
{ return shared_copy_back_insert_iterator<Container>(container); }


namespace Detail {
template<typename T>
struct GetPtrParam
{
	typedef typename GetSharedPtrParam<T>::ValueT ValueT;
	static shared_ptr<ValueT> ToShared(const T& t) { return t; }
};

template<typename T>
struct GetPtrParam<weak_ptr<T> >
{
	typedef T ValueT;
	static shared_ptr<ValueT> ToShared(const weak_ptr<T>& t) { return t.lock(); }
};
}

template<typename RawIteratorT>
struct SkippingPtrIterator : public iterator_base<SkippingPtrIterator<RawIteratorT>, typename Detail::GetPtrParam<typename RawIteratorT::value_type>, std::forward_iterator_tag>
{
	typedef iterator_base<SkippingPtrIterator<RawIteratorT>, typename Detail::GetPtrParam<typename RawIteratorT::value_type>::ValueT, std::forward_iterator_tag> base;
	typedef Detail::GetPtrParam<typename base::value_type> PtrGetterHelper;
private:
	typedef shared_ptr<typename base::value_type> LifetimeAssurance;
	LifetimeAssurance _lifetimeAssurance;
	RawIteratorT _currentIterator, _endIterator;
public:
	// Constructs end of sequence iterator
	SkippingPtrIterator(RawIteratorT last) :
		_currentIterator(last)
	{}
	SkippingPtrIterator(RawIteratorT first, RawIteratorT last) :
		_currentIterator(first),
		_endIterator(last)
	{ FindFirst(); }

	typename base::reference dereference() const
	{
		return *_lifetimeAssurance;
	}
	bool equal(const SkippingPtrIterator &other) const
	{ return _currentIterator == other._currentIterator; }
	void increment()
	{ FindNext(); }
private:
	bool AquireAssurance()
	{
		_lifetimeAssurance = PtrGetterHelper::ToShared(*_currentIterator);
		return _lifetimeAssurance;
	}
	void FindFirst()
	{
		while(_currentIterator != _endIterator && !AquireAssurance())
			++_currentIterator;
	}
	void FindNext()
	{
		CheckBounds();
		++_currentIterator;
		while(_currentIterator != _endIterator && !AquireAssurance())
			++_currentIterator;
	}
	inline void CheckBounds() const
	{
#if defined(DEBUG)
		STINGRAYKIT_CHECK(_currentIterator != _endIterator, IndexOutOfRangeException());
#endif
	}

	/** @} */
};

}

#endif

