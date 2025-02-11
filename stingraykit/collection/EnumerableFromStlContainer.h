#ifndef STINGRAYKIT_COLLECTION_ENUMERABLEFROMSTLCONTAINER_H
#define STINGRAYKIT_COLLECTION_ENUMERABLEFROMSTLCONTAINER_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
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
		{ using ValueT = typename RemoveConst<typename std::iterator_traits<IteratorType>::value_type>::ValueT; };

		template < typename ItemType, typename IteratorType, typename ContainerLifeAssuranceType = int >
		class IteratorsRangeEnumerator : public virtual IEnumerator<ItemType>
		{
			using IterableType = typename ValueTypeFromIteratorType<IteratorType>::ValueT;
			using ContainerLifeAssurancePtr = shared_ptr<ContainerLifeAssuranceType>;

		private:
			IteratorType				_current;
			IteratorType				_end;
			ContainerLifeAssurancePtr	_lifeAssurance;

		public:
			IteratorsRangeEnumerator(const IteratorType& begin, const IteratorType& end, const ContainerLifeAssurancePtr& lifeAssurance = null)
				: _current(begin), _end(end), _lifeAssurance(lifeAssurance)
			{ }

			bool Valid() const override
			{ return _current != _end; }

			ItemType Get() const override
			{
				STINGRAYKIT_CHECK(Valid(), "Enumerator is not valid!");
				return DoGet();
			}

			void Next() override
			{
				STINGRAYKIT_CHECK(Valid(), "Enumerator is not valid!");
				++_current;
			}

		private:
			template < typename ItemType__ = ItemType, typename EnableIf<IsConvertible<const IterableType&, ItemType__>::Value, int>::ValueT = 0 >
			ItemType__ DoGet() const
			{ return *_current; }

			template < typename ItemType__ = ItemType, typename EnableIf<!IsConvertible<const IterableType&, ItemType__>::Value, int>::ValueT = 0 >
			ItemType__ DoGet() const
			{ return ItemType__(*_current); }
		};

	}


	template < typename ContainerType >
	auto EnumeratorFromStlContainer(const ContainerType& container)
	{
		using IteratorType = typename ContainerType::const_iterator;
		using ItemType = typename Detail::ValueTypeFromIteratorType<IteratorType>::ValueT;
		return shared_ptr<IEnumerator<ItemType>>(make_shared_ptr<Detail::IteratorsRangeEnumerator<ItemType, IteratorType>>(container.begin(), container.end()));
	}

	template < typename ContainerType, typename ContainerLifeAssuranceType >
	auto EnumeratorFromStlContainer(const ContainerType& container, const shared_ptr<ContainerLifeAssuranceType>& lifeAssurance)
	{
		using IteratorType = typename ContainerType::const_iterator;
		using ItemType = typename Detail::ValueTypeFromIteratorType<IteratorType>::ValueT;
		return shared_ptr<IEnumerator<ItemType>>(make_shared_ptr<Detail::IteratorsRangeEnumerator<ItemType, IteratorType, ContainerLifeAssuranceType>>(container.begin(), container.end(), lifeAssurance));
	}

	template < typename ItemType, typename ContainerType >
	shared_ptr<IEnumerator<ItemType>> EnumeratorFromStlContainer(const ContainerType& container)
	{ return make_shared_ptr<Detail::IteratorsRangeEnumerator<ItemType, typename ContainerType::const_iterator>>(container.begin(), container.end()); }

	template < typename ItemType, typename ContainerType, typename ContainerLifeAssuranceType >
	shared_ptr<IEnumerator<ItemType>> EnumeratorFromStlContainer(const ContainerType& container, const shared_ptr<ContainerLifeAssuranceType>& lifeAssurance)
	{ return make_shared_ptr<Detail::IteratorsRangeEnumerator<ItemType, typename ContainerType::const_iterator, ContainerLifeAssuranceType>>(container.begin(), container.end(), lifeAssurance); }


	template < typename IteratorType >
	auto EnumeratorFromStlIterators(const IteratorType& begin, const IteratorType& end)
	{
		using ItemType = typename Detail::ValueTypeFromIteratorType<IteratorType>::ValueT;
		return shared_ptr<IEnumerator<ItemType>>(make_shared_ptr<Detail::IteratorsRangeEnumerator<ItemType, IteratorType>>(begin, end));
	}

	template < typename IteratorType, typename ContainerLifeAssuranceType >
	auto EnumeratorFromStlIterators(const IteratorType& begin, const IteratorType& end, const shared_ptr<ContainerLifeAssuranceType>& lifeAssurance)
	{
		using ItemType = typename Detail::ValueTypeFromIteratorType<IteratorType>::ValueT;
		return shared_ptr<IEnumerator<ItemType>>(make_shared_ptr<Detail::IteratorsRangeEnumerator<ItemType, IteratorType, ContainerLifeAssuranceType>>(begin, end, lifeAssurance));
	}

	template < typename ItemType, typename IteratorType >
	shared_ptr<IEnumerator<ItemType>> EnumeratorFromStlIterators(const IteratorType& begin, const IteratorType& end)
	{ return make_shared_ptr<Detail::IteratorsRangeEnumerator<ItemType, IteratorType>>(begin, end); }

	template < typename ItemType, typename IteratorType, typename ContainerLifeAssuranceType >
	shared_ptr<IEnumerator<ItemType>> EnumeratorFromStlIterators(const IteratorType& begin, const IteratorType& end, const shared_ptr<ContainerLifeAssuranceType>& lifeAssurance)
	{ return make_shared_ptr<Detail::IteratorsRangeEnumerator<ItemType, IteratorType, ContainerLifeAssuranceType>>(begin, end, lifeAssurance); }


	namespace Detail
	{

		template < typename ItemType, typename IteratorType, typename LifeAssuranceType = int >
		class StlIteratorsEnumerable : public virtual IEnumerable<ItemType>
		{
		private:
			IteratorType					_begin;
			IteratorType					_end;
			shared_ptr<LifeAssuranceType>	_lifeAssurance;

		public:
			StlIteratorsEnumerable(const IteratorType& begin, const IteratorType& end, const shared_ptr<LifeAssuranceType>& lifeAssurance = null)
				: _begin(begin), _end(end), _lifeAssurance(lifeAssurance)
			{ }

			shared_ptr<IEnumerator<ItemType>> GetEnumerator() const override
			{ return EnumeratorFromStlIterators<ItemType>(_begin, _end, _lifeAssurance); }
		};

	};


	template < typename ContainerType >
	auto EnumerableFromStlContainer(const ContainerType& container)
	{
		using IteratorType = typename ContainerType::const_iterator;
		using ItemType = typename Detail::ValueTypeFromIteratorType<IteratorType>::ValueT;
		return shared_ptr<IEnumerable<ItemType>>(make_shared_ptr<Detail::StlIteratorsEnumerable<ItemType, IteratorType>>(container.begin(), container.end()));
	}

	template < typename ContainerType, typename ContainerLifeAssuranceType >
	auto EnumerableFromStlContainer(const ContainerType& container, const shared_ptr<ContainerLifeAssuranceType>& lifeAssurance)
	{
		using IteratorType = typename ContainerType::const_iterator;
		using ItemType = typename Detail::ValueTypeFromIteratorType<IteratorType>::ValueT;
		return shared_ptr<IEnumerable<ItemType>>(make_shared_ptr<Detail::StlIteratorsEnumerable<ItemType, IteratorType, ContainerLifeAssuranceType>>(container.begin(), container.end(), lifeAssurance));
	}

	template < typename ItemType, typename ContainerType >
	shared_ptr<IEnumerable<ItemType>> EnumerableFromStlContainer(const ContainerType& container)
	{ return make_shared_ptr<Detail::StlIteratorsEnumerable<ItemType, typename ContainerType::const_iterator>>(container.begin(), container.end()); }

	template < typename ItemType, typename ContainerType, typename ContainerLifeAssuranceType >
	shared_ptr<IEnumerable<ItemType>> EnumerableFromStlContainer(const ContainerType& container, const shared_ptr<ContainerLifeAssuranceType>& lifeAssurance)
	{ return make_shared_ptr<Detail::StlIteratorsEnumerable<ItemType, typename ContainerType::const_iterator, ContainerLifeAssuranceType>>(container.begin(), container.end(), lifeAssurance); }


	template < typename IteratorType >
	auto EnumerableFromStlIterators(const IteratorType& begin, const IteratorType& end)
	{
		using ItemType = typename Detail::ValueTypeFromIteratorType<IteratorType>::ValueT;
		return shared_ptr<IEnumerable<ItemType>>(make_shared_ptr<Detail::StlIteratorsEnumerable<ItemType, IteratorType>>(begin, end));
	}

	template < typename IteratorType, typename ContainerLifeAssuranceType >
	auto EnumerableFromStlIterators(const IteratorType& begin, const IteratorType& end, const shared_ptr<ContainerLifeAssuranceType>& lifeAssurance)
	{
		using ItemType = typename Detail::ValueTypeFromIteratorType<IteratorType>::ValueT;
		return shared_ptr<IEnumerable<ItemType>>(make_shared_ptr<Detail::StlIteratorsEnumerable<ItemType, IteratorType, ContainerLifeAssuranceType>>(begin, end, lifeAssurance));
	}

	template < typename ItemType, typename IteratorType >
	shared_ptr<IEnumerable<ItemType>> EnumerableFromStlIterators(const IteratorType& begin, const IteratorType& end)
	{ return make_shared_ptr<Detail::StlIteratorsEnumerable<ItemType, IteratorType>>(begin, end); }

	template < typename ItemType, typename IteratorType, typename ContainerLifeAssuranceType >
	shared_ptr<IEnumerable<ItemType>> EnumerableFromStlIterators(const IteratorType& begin, const IteratorType& end, const shared_ptr<ContainerLifeAssuranceType>& lifeAssurance)
	{ return make_shared_ptr<Detail::StlIteratorsEnumerable<ItemType, IteratorType, ContainerLifeAssuranceType>>(begin, end, lifeAssurance); }

	/** @} */

}

#endif
