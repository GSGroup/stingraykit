#ifndef STINGRAYKIT_COLLECTION_ITERATORS_H
#define STINGRAYKIT_COLLECTION_ITERATORS_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/iterator_base.h>
#include <stingraykit/MetaProgramming.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template <typename T, size_t N>
	T* begin_iterator(T(&arr)[N])
	{ return &arr[0]; }


	template <typename T, size_t N>
	T* end_iterator(T(&arr)[N])
	{ return &arr[N]; }


	template <typename Collection>
	typename Collection::const_iterator begin_iterator(const Collection& collection)
	{ return collection.begin(); }


	template <typename Collection>
	typename Collection::const_iterator end_iterator(const Collection& collection)
	{ return collection.end(); }


	template <typename Collection>
	typename Collection::iterator begin_iterator(Collection& collection)
	{ return collection.begin(); }


	template <typename Collection>
	typename Collection::iterator end_iterator(Collection& collection)
	{ return collection.end(); }


	template<typename BidirectionalIteratorT>
	BidirectionalIteratorT prev_iterator(BidirectionalIteratorT it, typename std::iterator_traits<BidirectionalIteratorT>::difference_type n = 1)
	{ std::advance(it, -n);	return it; }


	template<typename BidirectionalIteratorT>
	BidirectionalIteratorT next_iterator(BidirectionalIteratorT it, typename std::iterator_traits<BidirectionalIteratorT>::difference_type n = 1)
	{ std::advance(it, n);	return it; }


	namespace Detail
	{
		template < typename IterType >
		struct GetMapKeysIteratorPointedType
		{
			typedef typename RemoveReference<typename IterType::value_type::first_type>::ValueT DerefKeyType;
			typedef typename If<IsConstReference<typename IterType::reference>::Value, const DerefKeyType, DerefKeyType>::ValueT ValueT;
		};


		template < typename IterType >
		struct GetMapValuesIteratorPointedType
		{
			typedef typename RemoveReference<typename IterType::value_type::second_type>::ValueT DerefValueType;
			typedef typename If<IsConstReference<typename IterType::reference>::Value, const DerefValueType, DerefValueType>::ValueT ValueT;
		};
	}

	template < typename MapIterator >
	class MapKeysIterator : public iterator_base<MapKeysIterator<MapIterator>, typename Detail::GetMapKeysIteratorPointedType<MapIterator>::ValueT, std::bidirectional_iterator_tag>
	{
		typedef iterator_base<MapKeysIterator<MapIterator>, typename Detail::GetMapKeysIteratorPointedType<MapIterator>::ValueT, std::bidirectional_iterator_tag>	BaseType;

		template < typename OtherMapIterator >
		friend class MapKeysIterator;

	private:
		MapIterator		_wrapped;

	public:
		MapKeysIterator(const MapIterator& wrapped) : _wrapped(wrapped) { }

		template < typename OtherMapIterator >
		MapKeysIterator(const MapKeysIterator<OtherMapIterator>& other) : _wrapped(other._wrapped) { }

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

		template < typename OtherMapIterator >
		friend class MapValuesIterator;

	private:
		MapIterator		_wrapped;

	public:
		MapValuesIterator(const MapIterator& wrapped) : _wrapped(wrapped) { }

		template < typename OtherMapIterator >
		MapValuesIterator(const MapValuesIterator<OtherMapIterator>& other) : _wrapped(other._wrapped) { }

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

	/** @} */

}


#endif
