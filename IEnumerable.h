#ifndef STINGRAY_TOOLKIT_IENUMERABLE_H
#define STINGRAY_TOOLKIT_IENUMERABLE_H


#include <stingray/toolkit/IEnumerator.h>

#define TOOLKIT_DECLARE_ENUMERABLE(ClassName) \
		typedef stingray::IEnumerable<ClassName>				ClassName##Enumerable; \
		TOOLKIT_DECLARE_PTR(ClassName##Enumerable); \
		TOOLKIT_DECLARE_ENUMERATOR(ClassName)

namespace stingray
{


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


	template < typename T >
	struct EmptyEnumerable : public virtual IEnumerable<T>
	{
		virtual shared_ptr<IEnumerator<T> > GetEnumerator() const
		{ return make_shared<EmptyEnumerator<T> >(); }
	};


	class EmptyEnumerableProxy
	{
	public:
		template< typename U >
		operator shared_ptr<IEnumerable<U> >() const
		{ return make_shared<EmptyEnumerable<U> >(); }
	};


	inline EmptyEnumerableProxy MakeEmptyEnumerable()
	{ return EmptyEnumerableProxy(); }


	template < typename T >
	struct OneItemEnumerable : public virtual IEnumerable<T>
	{
	private:
		T		_value;

	public:
		OneItemEnumerable(typename GetConstReferenceType<T>::ValueT value)
			: _value(value)
		{ }

		virtual shared_ptr<IEnumerator<T> > GetEnumerator() const
		{ return make_shared<OneItemEnumerator<T> >(_value); }
	};


	template< typename T >
	class OneItemEnumerableProxy
	{
	private:
		T	_item;

	public:
		explicit OneItemEnumerableProxy(const T& item)
			: _item(item)
		{ }

		template< typename U >
		operator shared_ptr<IEnumerable<U> >() const
		{ return make_shared<OneItemEnumerable<U> >(_item); }
	};


	template< typename T >
	OneItemEnumerableProxy<T> MakeOneItemEnumerable(const T& item)
	{ return OneItemEnumerableProxy<T>(item); }


	template < typename SrcType, typename DestType >
	class EnumerableWrapper : public virtual IEnumerable<DestType>
	{
		typedef shared_ptr<IEnumerable<SrcType> >					SrcEnumerablePtr;
		typedef typename GetConstReferenceType<SrcType>::ValueT		ConstSrcTypeRef;
		typedef function< bool(ConstSrcTypeRef) >					SkipPredicateType;
		typedef function< DestType (ConstSrcTypeRef) >				CastPredicateType;

	private:
		SrcEnumerablePtr			_srcEnumerable;
		CastPredicateType			_castPredicate;
		SkipPredicateType			_skipPredicate;

	public:
		EnumerableWrapper(const SrcEnumerablePtr& srcEnumerable)
			: _srcEnumerable(srcEnumerable), _castPredicate(&EnumerableWrapper::DefaultCast), _skipPredicate(&EnumerableWrapper::NoSkip)
		{}

		EnumerableWrapper(const SrcEnumerablePtr& srcEnumerable, const CastPredicateType& castPredicate)
			: _srcEnumerable(srcEnumerable), _castPredicate(castPredicate), _skipPredicate(&EnumerableWrapper::NoSkip)
		{}

		EnumerableWrapper(const SrcEnumerablePtr& srcEnumerable, const CastPredicateType& castPredicate, const SkipPredicateType& skipPredicate)
			: _srcEnumerable(srcEnumerable), _castPredicate(castPredicate), _skipPredicate(skipPredicate)
		{}

		virtual shared_ptr<IEnumerator<DestType> > GetEnumerator() const
		{ return WrapEnumerator(_srcEnumerable->GetEnumerator(), _castPredicate, _skipPredicate); }

	private:
		static bool NoSkip(ConstSrcTypeRef) { return false; }
		static DestType DefaultCast(ConstSrcTypeRef src) { return DestType(src); }
	};


	template<typename SrcEnumerableType, typename CasterType>
	shared_ptr<IEnumerable<typename function_info<CasterType>::RetType> > WrapEnumerable(const shared_ptr<SrcEnumerableType>& src, const CasterType& caster)
	{ return make_shared<EnumerableWrapper<typename SrcEnumerableType::ItemType, typename function_info<CasterType>::RetType> >(src, caster); }


	template<typename SrcEnumerableType, typename CasterType, typename SkipperType>
	shared_ptr<IEnumerable<typename function_info<CasterType>::RetType> > WrapEnumerable(const shared_ptr<SrcEnumerableType>& src, const CasterType& caster, const SkipperType& skipper)
	{ return make_shared<EnumerableWrapper<typename SrcEnumerableType::ItemType, typename function_info<CasterType>::RetType> >(src, caster, skipper); }


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
				CastProxy(const SrcEnumerablePtr& srcEnumerable) : base(srcEnumerable, &CastProxy::Cast, &CastProxy::Skip)
				{ }

			private:
				static DestType Cast(ConstSrcTypeRef src)	{ return dynamic_caster(src); }
				static bool Skip(ConstSrcTypeRef src)		{ return !InstanceOf<typename GetSharedPtrParam<DestType>::ValueT>(src); }
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


}


#endif
