#ifndef STINGRAY_TOOLKIT_ITERATORS_H
#define STINGRAY_TOOLKIT_ITERATORS_H


#include <stingray/toolkit/MetaProgramming.h>
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
		typedef iterator_base<MapKeysIterator<MapIterator>, typename Detail::GetMapKeysIteratorPointedType<MapIterator>::ValueT, std::bidirectional_iterator_tag>	BaseType;

	private:
		MapIterator		_wrapped;

	public:
		MapKeysIterator(const MapIterator& wrapped) : _wrapped(wrapped) { }

		typename BaseType::reference dereference() const	{ return _wrapped->first; }
		bool equal(const MapKeysIterator& other) const		{ return _wrapped == other._wrapped; }
		void increment()									{ ++_wrapped; }
		void decrement()									{ --_wrapped; }

		MapIterator base() const							{ return _wrapped; }
	};

	template < typename MapIterator >
	class MapValuesIterator : public iterator_base<MapValuesIterator<MapIterator>, typename Detail::GetMapValuesIteratorPointedType<MapIterator>::ValueT, std::bidirectional_iterator_tag>
	{
		typedef iterator_base<MapValuesIterator<MapIterator>, typename Detail::GetMapValuesIteratorPointedType<MapIterator>::ValueT, std::bidirectional_iterator_tag>	BaseType;

	private:
		MapIterator		_wrapped;

	public:
		MapValuesIterator(const MapIterator& wrapped) : _wrapped(wrapped) { }
		MapValuesIterator& operator =(const MapIterator& w)	{ _wrapped = w; return *this; }

		typename BaseType::reference dereference() const	{ return _wrapped->second; }
		bool equal(const MapValuesIterator& other) const	{ return _wrapped == other._wrapped; }
		void increment()									{ ++_wrapped; }
		void decrement()									{ --_wrapped; }

		MapIterator base() const							{ return _wrapped; }
	};


	template < typename MapIterator >
	MapKeysIterator<MapIterator> keys_iterator(const MapIterator& srcIter)
	{ return MapKeysIterator<MapIterator>(srcIter); }

	template < typename MapIterator >
	MapValuesIterator<MapIterator> values_iterator(const MapIterator& srcIter)
	{ return MapValuesIterator<MapIterator>(srcIter); }



	template < typename Map >
	class MapUpdateIterator : public iterator_base<MapUpdateIterator<Map>, typename Map::value_type, std::output_iterator_tag, typename Map::difference_type, typename Map::pointer, MapUpdateIterator<Map>&>
	{
	private:
		Map*	_map;

	public:
		explicit MapUpdateIterator(Map& map) : _map(&map) { }

		MapUpdateIterator& dereference()	{ return *this; }
		void increment()					{ }

		MapUpdateIterator& operator= (const typename Map::const_reference value)
		{
			(*_map)[value.first] = value.second;
			return *this;
		}
	};


	template < typename Map >
	MapUpdateIterator<Map> update_iterator(Map& map)
	{ return MapUpdateIterator<Map>(map); }



	template < typename WrappedIterator >
	class weak_locker_iterator :
		public iterator_base<
			weak_locker_iterator<WrappedIterator>,
			shared_ptr<typename GetWeakPtrParam<typename std::iterator_traits<WrappedIterator>::value_type>::ValueT>,
			std::forward_iterator_tag,
			std::ptrdiff_t,
			const shared_ptr<typename GetWeakPtrParam<typename std::iterator_traits<WrappedIterator>::value_type>::ValueT>*,
			const shared_ptr<typename GetWeakPtrParam<typename std::iterator_traits<WrappedIterator>::value_type>::ValueT>&>
	{
		typedef iterator_base<
			weak_locker_iterator<WrappedIterator>,
			shared_ptr<typename GetWeakPtrParam<typename std::iterator_traits<WrappedIterator>::value_type>::ValueT>,
			std::forward_iterator_tag,
			std::ptrdiff_t,
			const shared_ptr<typename GetWeakPtrParam<typename std::iterator_traits<WrappedIterator>::value_type>::ValueT>*,
			const shared_ptr<typename GetWeakPtrParam<typename std::iterator_traits<WrappedIterator>::value_type>::ValueT>&>		base;
		typedef typename GetWeakPtrParam<typename std::iterator_traits<WrappedIterator>::value_type>::ValueT						T;

	private:
		WrappedIterator		_it;
		WrappedIterator		_end;
		bool				_valid;
		shared_ptr<T>		_shared;

	public:
		weak_locker_iterator(const WrappedIterator& begin, const WrappedIterator& end)
			: _it(begin), _end(end), _valid(true)
		{ FindNextValid(); }

		weak_locker_iterator()
			: _valid(false)
		{ }

	 	typename base::reference dereference() const		{ return _shared; }
	 	bool equal(const weak_locker_iterator& other) const
		{
			return
				(_it == _end && !other._valid) ||
				(other._it == other._end && !_valid) ||
				_it == other._it;
		}

	 	void increment()
		{
			TOOLKIT_CHECK(_it != _end, "Cannot increment iterator");
			++_it;
			FindNextValid();
		}

		operator bool() const { return _it != _end; }

	private:
		void FindNextValid()
		{
			while (_it != _end)
			{
				_shared = _it->lock();
				if (_shared)
					return;
				++_it;
			}
		}
	};

	template < typename WrappedIterator >
	weak_locker_iterator<WrappedIterator> weak_locker(const WrappedIterator& begin, const WrappedIterator& end)
	{ return weak_locker_iterator<WrappedIterator>(begin, end); }

	template < typename WrappedIterator >
	weak_locker_iterator<WrappedIterator> weak_locker_end(const WrappedIterator&)
	{ return weak_locker_iterator<WrappedIterator>(); }

}


#endif
