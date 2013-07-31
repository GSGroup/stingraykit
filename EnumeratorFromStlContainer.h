#ifndef STINGRAY_TOOLKIT_ENUMERATORFROMSTLCONTAINER_H
#define STINGRAY_TOOLKIT_ENUMERATORFROMSTLCONTAINER_H

#include <iterator>

#include <stingray/toolkit/IEnumerable.h>
#include <stingray/toolkit/IEnumerator.h>
#include <stingray/toolkit/PrivateIncludeGuard.h>
#include <stingray/toolkit/function.h>
#include <stingray/toolkit/shared_ptr.h>


namespace stingray
{

	/*! \cond GS_INTERNAL */

	namespace Detail
	{
		template < typename IteratorType >
		struct ValueTypeFromIteratorType
		{ typedef typename Deconst<typename std::iterator_traits<IteratorType>::value_type>::ValueT	ValueT; };

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
		virtual ValueType Get() const	{ TOOLKIT_CHECK(Valid(), "Enumerator is not valid!"); return *_current; }
		virtual void Next()				{ TOOLKIT_CHECK(Valid(), "Enumerator is not valid!"); ++_current; }
	};

	/*! \endcond */


	template < typename ContainerType >
	shared_ptr< typename Detail::EnumeratorTypeFromIteratorType<typename ContainerType::const_iterator>::ValueT >
		EnumeratorFromStlContainer(const ContainerType& container)
	{
		typedef typename ContainerType::const_iterator	IteratorType;
		return make_shared<IteratorsRangeEnumerator<IteratorType> >(container.begin(), container.end());
	}

	template < typename ContainerType, typename ContainerLifeAssuranceType >
	shared_ptr< typename Detail::EnumeratorTypeFromIteratorType<typename ContainerType::const_iterator>::ValueT >
		EnumeratorFromStlContainer(const ContainerType& container, shared_ptr<ContainerLifeAssuranceType> lifeAssurance)
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
		EnumeratorFromStlIterators(const IteratorType& begin, const IteratorType& end, shared_ptr<ContainerLifeAssuranceType> lifeAssurance)
	{ return make_shared<IteratorsRangeEnumerator<IteratorType, ContainerLifeAssuranceType> >(begin, end, lifeAssurance); }


	template<typename ResultType, typename SourceType>
	class AdapterEnumerator : public virtual IEnumerator<ResultType>
	{
		typedef function<ResultType(const SourceType& src)>	AdapterType;

	private:
		AdapterType _adapter;
		const SourceType *_current, *_end;

	public:
		AdapterEnumerator(const AdapterType& adapter, const SourceType *begin, const SourceType *end)
			: _adapter(adapter), _current(begin), _end(end)
		{ }

		virtual bool Valid() const { return _current != _end; };
		virtual ResultType Get() const { return _adapter(*_current); }

		virtual void Next() {
			if (_current != _end)
				++_current;
		}
	};

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
		EnumerableFromStlContainer(const ContainerType& container, shared_ptr<ContainerLifeAssuranceType> lifeAssurance)
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
		EnumerableFromStlIterators(const IteratorType& begin, const IteratorType& end, shared_ptr<ContainerLifeAssuranceType> lifeAssurance)
	{ return make_shared<Detail::StlIteratorsEnumerable<IteratorType, ContainerLifeAssuranceType> >(begin, end, lifeAssurance); }



}


#endif
