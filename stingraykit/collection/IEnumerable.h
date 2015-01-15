#ifndef STINGRAYKIT_COLLECTION_IENUMERABLE_H
#define STINGRAYKIT_COLLECTION_IENUMERABLE_H


#include <stingraykit/collection/IEnumerator.h>

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
