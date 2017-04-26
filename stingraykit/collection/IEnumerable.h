#ifndef STINGRAYKIT_COLLECTION_IENUMERABLE_H
#define STINGRAYKIT_COLLECTION_IENUMERABLE_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumeratorWrapper.h>
#include <stingraykit/collection/IEnumerator.h>
#include <stingraykit/collection/RangeBase.h>
#include <stingraykit/collection/ToEnumerator.h>
#include <stingraykit/collection/ToRange.h>

#define STINGRAYKIT_DECLARE_ENUMERABLE(ClassName) \
		typedef stingray::IEnumerable<ClassName>				ClassName##Enumerable; \
		STINGRAYKIT_DECLARE_PTR(ClassName##Enumerable); \
		STINGRAYKIT_DECLARE_ENUMERATOR(ClassName)

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename T >
	struct IEnumerable
	{
		typedef T ItemType;

		virtual ~IEnumerable() { }

		virtual shared_ptr<IEnumerator<T> > GetEnumerator() const = 0;
	};


	template < typename T >
	struct IReversableEnumerable
	{
		virtual ~IReversableEnumerable() { }

		virtual shared_ptr<IEnumerable<T> > Reverse() const = 0;
	};


	template < typename T >
	struct IsEnumerable
	{
	private:
		template < typename U >
		static YesType GetIsEnumerable(const IEnumerable<U>*);
		static NoType GetIsEnumerable(...);

	public:
		static const bool Value = sizeof(GetIsEnumerable((const T*)0)) == sizeof(YesType);
	};


	namespace Detail
	{
		template< typename T >
		struct ToEnumeratorImpl<T, typename EnableIf<IsEnumerable<T>::Value, void>::ValueT>
		{
			typedef IEnumerator<typename T::ItemType>	ValueT;

			static shared_ptr<ValueT> Do(const shared_ptr<T>& src) { return src->GetEnumerator(); }
		};


		template <typename Enumerable_>
		class EnumerableToRange : public Range::RangeBase<EnumerableToRange<Enumerable_>, typename Enumerable_::ItemType, std::forward_iterator_tag>
		{
			typedef Range::RangeBase<EnumerableToRange<Enumerable_>, typename Enumerable_::ItemType, std::forward_iterator_tag> base;
			typedef EnumerableToRange<Enumerable_> Self;

		private:
			const Enumerable_&                                       _enumerable;
			shared_ptr<IEnumerator<typename Enumerable_::ItemType> > _enumerator;

		public:
			EnumerableToRange(const Enumerable_& e) : _enumerable(e)
			{ First(); }

			bool Valid() const             { return _enumerator->Valid(); }
			typename base::ValueType Get() { return _enumerator->Get(); }

			Self& First()                  { _enumerator = _enumerable.GetEnumerator(); return *this; }
			Self& Next()                   { _enumerator->Next(); return *this; }
		};


		template <typename Enumerable_>
		struct ToRangeImpl<Enumerable_, typename EnableIf<IsEnumerable<Enumerable_>::Value, void>::ValueT>
		{
			typedef EnumerableToRange<Enumerable_> ValueT;

			static ValueT Do(const Enumerable_& r)
			{ return ValueT(r); }
		};
	}


	template < typename SrcType, typename DestType >
	class EnumerableWrapper : public virtual IEnumerable<DestType>
	{
		typedef shared_ptr<IEnumerable<SrcType> >					SrcEnumerablePtr;
		typedef typename GetConstReferenceType<SrcType>::ValueT		ConstSrcTypeRef;
		typedef function< bool(ConstSrcTypeRef) >					FilterPredicate;
		typedef function< DestType (ConstSrcTypeRef) >				Caster;

	private:
		SrcEnumerablePtr			_srcEnumerable;
		Caster						_caster;
		FilterPredicate				_filterPredicate;

	public:
		EnumerableWrapper(const SrcEnumerablePtr& srcEnumerable)
			: _srcEnumerable(srcEnumerable), _caster(&EnumerableWrapper::DefaultCast), _filterPredicate(&EnumerableWrapper::NoFilter)
		{}

		EnumerableWrapper(const SrcEnumerablePtr& srcEnumerable, const Caster& caster)
			: _srcEnumerable(srcEnumerable), _caster(caster), _filterPredicate(&EnumerableWrapper::NoFilter)
		{}

		EnumerableWrapper(const SrcEnumerablePtr& srcEnumerable, const Caster& caster, const FilterPredicate& filterPredicate)
			: _srcEnumerable(srcEnumerable), _caster(caster), _filterPredicate(filterPredicate)
		{}

		virtual shared_ptr<IEnumerator<DestType> > GetEnumerator() const
		{ return WrapEnumerator(_srcEnumerable->GetEnumerator(), _caster, _filterPredicate); }

	private:
		static DestType DefaultCast(ConstSrcTypeRef src)	{ return DestType(src); }
		static bool NoFilter(ConstSrcTypeRef)				{ return true; }
	};


	template<typename SrcEnumerableType, typename CasterType>
	shared_ptr<IEnumerable<typename function_info<CasterType>::RetType> > WrapEnumerable(const shared_ptr<SrcEnumerableType>& src, const CasterType& caster)
	{ return make_shared<EnumerableWrapper<typename SrcEnumerableType::ItemType, typename function_info<CasterType>::RetType> >(src, caster); }


	template<typename SrcEnumerableType, typename CasterType, typename FilterPredicate>
	shared_ptr<IEnumerable<typename function_info<CasterType>::RetType> > WrapEnumerable(const shared_ptr<SrcEnumerableType>& src, const CasterType& caster, const FilterPredicate& filterPredicate)
	{ return make_shared<EnumerableWrapper<typename SrcEnumerableType::ItemType, typename function_info<CasterType>::RetType> >(src, caster, filterPredicate); }


	namespace Detail
	{
		template <typename SrcType>
		class EnumerableCaster
		{
			typedef shared_ptr<IEnumerable<SrcType> >					SrcEnumerablePtr;

			template < typename DestType >
			class CastProxy : public EnumerableWrapper<SrcType, DestType>
			{
				typedef EnumerableWrapper<SrcType, DestType>				base;
				typedef typename GetConstReferenceType<SrcType>::ValueT		ConstSrcTypeRef;

			public:
				CastProxy(const SrcEnumerablePtr& srcEnumerable) : base(srcEnumerable, &CastProxy::Cast, InstanceOfPredicate<typename GetSharedPtrParam<DestType>::ValueT>())
				{ }

			private:
				static DestType Cast(ConstSrcTypeRef src)	{ return dynamic_caster(src); }
			};

		private:
			SrcEnumerablePtr			_srcEnumerable;

		public:
			EnumerableCaster(const SrcEnumerablePtr& srcEnumerable) : _srcEnumerable(srcEnumerable)
			{ }

			template < typename DestType >
			operator shared_ptr<IEnumerable<DestType> > () const
			{ return make_shared<CastProxy<DestType> >(_srcEnumerable); }
		};
	}


	template < typename T >
	typename Detail::EnumerableCaster<typename T::ItemType> GetEnumerableCaster(const shared_ptr<T>& enumerable)
	{ return Detail::EnumerableCaster<typename T::ItemType>(enumerable); }

	/** @} */

}


#endif
