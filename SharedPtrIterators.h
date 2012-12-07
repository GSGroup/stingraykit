#ifndef STINGRAY_TOOLKIT_SHAREDPTRITERATORS_H
#define STINGRAY_TOOLKIT_SHAREDPTRITERATORS_H

#include <stingray/toolkit/shared_ptr.h>

namespace stingray
{


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
FORCE_INLINE shared_copy_back_insert_iterator<Container> shared_copy_back_inserter(Container& container)
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
	FORCE_INLINE void CheckBounds() const
	{
#if defined(DEBUG)
		TOOLKIT_CHECK(_currentIterator != _endIterator, IndexOutOfRangeException());
#endif
	}
};

}

#endif

