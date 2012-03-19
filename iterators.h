#ifndef __GS_DVRLIB_TOOLKIT_ITERATORS_H__
#define __GS_DVRLIB_TOOLKIT_ITERATORS_H__


#include <stingray/toolkit/iterator_base.h>


namespace stingray
{

	template<typename BidirectionalIteratorT>
	BidirectionalIteratorT prev(BidirectionalIteratorT it, typename std::iterator_traits<BidirectionalIteratorT>::difference_type n = 1)
	{ std::advance(it, -n);	return it; }

	template<typename BidirectionalIteratorT>
	BidirectionalIteratorT next(BidirectionalIteratorT it, typename std::iterator_traits<BidirectionalIteratorT>::difference_type n = 1)
	{ std::advance(it, n);	return it; }

	namespace Detail
	{
		template < typename IterType >
		struct GetMapKeysIteratorPointedType : public If<IsConstReference<typename IterType::reference>::Value, const typename IterType::value_type::first_type, typename IterType::value_type::first_type>
		{ };

		template < typename IterType >
		struct GetMapValuesIteratorPointedType : public If<IsConstReference<typename IterType::reference>::Value, const typename IterType::value_type::second_type, typename IterType::value_type::second_type>
		{ };
	}

	template < typename MapIterator >
	class MapKeysIterator : public iterator_base<MapKeysIterator<MapIterator>, typename Detail::GetMapKeysIteratorPointedType<MapIterator>::ValueT, std::bidirectional_iterator_tag>
	{
		typedef iterator_base<MapKeysIterator<MapIterator>, typename Detail::GetMapKeysIteratorPointedType<MapIterator>::ValueT, std::bidirectional_iterator_tag>	base;

	private:
		MapIterator		_wrapped;

	public:
		MapKeysIterator(const MapIterator& wrapped) : _wrapped(wrapped) { }

		typename base::reference dereference() const	{ return _wrapped->first; }
		bool equal(const MapKeysIterator& other) const	{ return _wrapped == other._wrapped; }
		void increment()								{ ++_wrapped; }
		void decrement()								{ --_wrapped; }
	};

	template < typename MapIterator >
	class MapValuesIterator : public iterator_base<MapValuesIterator<MapIterator>, typename Detail::GetMapValuesIteratorPointedType<MapIterator>::ValueT, std::bidirectional_iterator_tag>
	{
		typedef iterator_base<MapValuesIterator<MapIterator>, typename Detail::GetMapValuesIteratorPointedType<MapIterator>::ValueT, std::bidirectional_iterator_tag>	base;

	private:
		MapIterator		_wrapped;

	public:
		MapValuesIterator(const MapIterator& wrapped) : _wrapped(wrapped) { }

		typename base::reference dereference() const		{ return _wrapped->second; }
		bool equal(const MapValuesIterator& other) const	{ return _wrapped == other._wrapped; }
		void increment()									{ ++_wrapped; }
		void decrement()									{ --_wrapped; }
	};


	template < typename MapIterator >
	MapKeysIterator<MapIterator> keys_iterator(const MapIterator& srcIter)
	{ return MapKeysIterator<MapIterator>(srcIter); }

	template < typename MapIterator >
	MapValuesIterator<MapIterator> values_iterator(const MapIterator& srcIter)
	{ return MapValuesIterator<MapIterator>(srcIter); }

}


#endif
