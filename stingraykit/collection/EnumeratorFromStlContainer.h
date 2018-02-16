#ifndef STINGRAYKIT_COLLECTION_ENUMERATORFROMSTLCONTAINER_H
#define STINGRAYKIT_COLLECTION_ENUMERATORFROMSTLCONTAINER_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/IEnumerable.h>

#include <iterator>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	namespace Detail
	{
		template < typename IteratorType >
		struct ValueTypeFromIteratorType
		{ typedef typename RemoveConst<typename std::iterator_traits<IteratorType>::value_type>::ValueT	ValueT; };

		template < typename IteratorType >
		struct EnumeratorTypeFromIteratorType
		{ typedef IEnumerator<typename ValueTypeFromIteratorType<IteratorType>::ValueT>	ValueT; };

		template < typename IteratorType >
		struct EnumerableTypeFromIteratorType
		{ typedef IEnumerable<typename ValueTypeFromIteratorType<IteratorType>::ValueT>	ValueT; };
	}

	template < typename IteratorType, typename ContainerLifeAssuranceType = int >
	class IteratorsRangeEnumerator : public virtual Detail::EnumeratorTypeFromIteratorType<IteratorType>::ValueT
	{
		typedef typename Detail::ValueTypeFromIteratorType<IteratorType>::ValueT	ValueType;
		typedef shared_ptr<ContainerLifeAssuranceType>								ContainerLifeAssurancePtr;

	private:
		IteratorType				_current;
		IteratorType				_end;
		ContainerLifeAssurancePtr	_lifeAssurance;

	public:
		IteratorsRangeEnumerator(const IteratorType& begin, const IteratorType& end, const ContainerLifeAssurancePtr& lifeAssurance = ContainerLifeAssurancePtr())
			: _current(begin), _end(end), _lifeAssurance(lifeAssurance)
		{ }

		virtual bool Valid() const		{ return _current != _end; }
		virtual ValueType Get() const	{ STINGRAYKIT_CHECK(Valid(), "Enumerator is not valid!"); return *_current; }
		virtual void Next()				{ STINGRAYKIT_CHECK(Valid(), "Enumerator is not valid!"); ++_current; }
	};



	template < typename ContainerType >
	shared_ptr< typename Detail::EnumeratorTypeFromIteratorType<typename ContainerType::const_iterator>::ValueT >
		EnumeratorFromStlContainer(const ContainerType& container)
	{
		typedef typename ContainerType::const_iterator	IteratorType;
		return make_shared<IteratorsRangeEnumerator<IteratorType> >(container.begin(), container.end());
	}

	template < typename ContainerType, typename ContainerLifeAssuranceType >
	shared_ptr< typename Detail::EnumeratorTypeFromIteratorType<typename ContainerType::const_iterator>::ValueT >
		EnumeratorFromStlContainer(const ContainerType& container, const shared_ptr<ContainerLifeAssuranceType>& lifeAssurance)
	{
		typedef typename ContainerType::const_iterator	IteratorType;
		return make_shared<IteratorsRangeEnumerator<IteratorType, ContainerLifeAssuranceType> >(container.begin(), container.end(), lifeAssurance);
	}


	template < typename IteratorType >
	shared_ptr< typename Detail::EnumeratorTypeFromIteratorType<IteratorType>::ValueT >
		EnumeratorFromStlIterators(const IteratorType& begin, const IteratorType& end)
	{ return make_shared<IteratorsRangeEnumerator<IteratorType> >(begin, end); }

	template < typename IteratorType, typename ContainerLifeAssuranceType >
	shared_ptr< typename Detail::EnumeratorTypeFromIteratorType<IteratorType>::ValueT >
		EnumeratorFromStlIterators(const IteratorType& begin, const IteratorType& end, const shared_ptr<ContainerLifeAssuranceType>& lifeAssurance)
	{ return make_shared<IteratorsRangeEnumerator<IteratorType, ContainerLifeAssuranceType> >(begin, end, lifeAssurance); }


	namespace Detail
	{

		template < typename IteratorType, typename LifeAssuranceType = int>
		class StlIteratorsEnumerable : public virtual EnumerableTypeFromIteratorType<IteratorType>::ValueT
		{
			typedef typename ValueTypeFromIteratorType<IteratorType>::ValueT		ValueType;

		private:
			IteratorType					_begin;
			IteratorType					_end;
			shared_ptr<LifeAssuranceType>	_lifeAssurance;

		public:
			StlIteratorsEnumerable(const IteratorType& begin, const IteratorType& end, const shared_ptr<LifeAssuranceType>& lifeAssurance = null)
				: _begin(begin), _end(end), _lifeAssurance(lifeAssurance)
			{ }

			virtual shared_ptr<IEnumerator<ValueType> > GetEnumerator() const
			{ return EnumeratorFromStlIterators(_begin, _end, _lifeAssurance); }
		};

	};

	template < typename ContainerType >
	shared_ptr< typename Detail::EnumerableTypeFromIteratorType<typename ContainerType::const_iterator>::ValueT >
		EnumerableFromStlContainer(const ContainerType& container)
	{
		typedef typename ContainerType::const_iterator	IteratorType;
		return make_shared<Detail::StlIteratorsEnumerable<IteratorType> >(container.begin(), container.end());
	}

	template < typename ContainerType, typename ContainerLifeAssuranceType >
	shared_ptr< typename Detail::EnumerableTypeFromIteratorType<typename ContainerType::const_iterator>::ValueT >
		EnumerableFromStlContainer(const ContainerType& container, const shared_ptr<ContainerLifeAssuranceType>& lifeAssurance)
	{
		typedef typename ContainerType::const_iterator	IteratorType;
		return make_shared<Detail::StlIteratorsEnumerable<IteratorType, ContainerLifeAssuranceType> >(container.begin(), container.end(), lifeAssurance);
	}

	template < typename IteratorType >
	shared_ptr< typename Detail::EnumerableTypeFromIteratorType<IteratorType>::ValueT >
		EnumerableFromStlIterators(const IteratorType& begin, const IteratorType& end)
	{ return make_shared<Detail::StlIteratorsEnumerable<IteratorType> >(begin, end); }

	template < typename IteratorType, typename ContainerLifeAssuranceType >
	shared_ptr< typename Detail::EnumerableTypeFromIteratorType<IteratorType>::ValueT >
		EnumerableFromStlIterators(const IteratorType& begin, const IteratorType& end, const shared_ptr<ContainerLifeAssuranceType>& lifeAssurance)
	{ return make_shared<Detail::StlIteratorsEnumerable<IteratorType, ContainerLifeAssuranceType> >(begin, end, lifeAssurance); }


	/** @} */

}


#endif
