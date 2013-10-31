#ifndef STINGRAY_TOOLKIT_IENUMERATOR_H
#define STINGRAY_TOOLKIT_IENUMERATOR_H


#include <stingray/toolkit/MetaProgramming.h>
#include <stingray/toolkit/dynamic_caster.h>
#include <stingray/toolkit/function.h>
#include <stingray/toolkit/shared_ptr.h>


#define TOOLKIT_DECLARE_ENUMERATOR(ClassName) \
		typedef stingray::IEnumerator<ClassName>				ClassName##Enumerator; \
		TOOLKIT_DECLARE_PTR(ClassName##Enumerator)

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename T >
	struct IEnumerator
	{
		typedef T		ItemType;

		virtual ~IEnumerator() { }

		virtual bool Valid() const = 0;
		virtual T Get() const = 0;
		virtual void Next() = 0;
	};


	template < typename T >
	struct IsEnumerator
	{
	private:
		template < typename U >
		static YesType GetIsEnumerator(const IEnumerator<U>*);
		static NoType GetIsEnumerator(...);

	public:
		static const bool Value = sizeof(GetIsEnumerator((const T*)0)) == sizeof(YesType);
	};


	/**
	 * @brief A foreach loop
	 * @param[in] ... &lt;type&gt; &lt;name&gt; IN &lt;enumerable or enumerator&gt; [ WHERE &lt;some condition&gt; ]
	 * @par Example:
	 * @code
	 * FOR_EACH(ISomeObjectPtr obj IN something->GetObjectCollection())
	 * {
	 *     ISomeOtherObjectPtr other_obj = dynamic_pointer_cast<ISomeOtherObject>(obj);
	 *     if (!other_obj)
	 *         continue;
	 *     if (other_obj->GetProperty() == 1)
	 *         continue;
	 *     other_obj->SomeMethod();
	 * }
	 * // or
	 * FOR_EACH(ISomeOtherObjectPtr other_obj IN something->GetObjectCollection() WHERE other_obj->GetProperty() != 1)
	 *     other_obj->SomeMethod();
	 * @endcode
	 */
#define FOR_EACH(...) FOR_EACH__IMPL(__VA_ARGS__)


	template < typename SrcType, typename DestType >
	class EnumeratorWrapper : public IEnumerator<DestType>
	{
		typedef shared_ptr<IEnumerator<SrcType> >					SrcEnumeratorPtr;
		typedef typename GetConstReferenceType<SrcType>::ValueT		ConstSrcTypeRef;
		typedef function< bool(ConstSrcTypeRef) >					SkipPredicateType;
		typedef function< DestType (ConstSrcTypeRef) >				CastPredicateType;

	private:
		SrcEnumeratorPtr			_srcEnumerator;
		CastPredicateType			_castPredicate;
		SkipPredicateType			_skipPredicate;

	public:
		EnumeratorWrapper(const SrcEnumeratorPtr& srcEnumerator)
			: _srcEnumerator(srcEnumerator), _castPredicate(&EnumeratorWrapper::DefaultCast), _skipPredicate(&EnumeratorWrapper::NoSkip)
		{ FindFirst(); }

		EnumeratorWrapper(const SrcEnumeratorPtr& srcEnumerator, const CastPredicateType& castPredicate)
			: _srcEnumerator(srcEnumerator), _castPredicate(castPredicate), _skipPredicate(&EnumeratorWrapper::NoSkip)
		{ FindFirst(); }

		EnumeratorWrapper(const SrcEnumeratorPtr& srcEnumerator, const CastPredicateType& castPredicate, const SkipPredicateType& skipPredicate)
			: _srcEnumerator(srcEnumerator), _castPredicate(castPredicate), _skipPredicate(skipPredicate)
		{ FindFirst(); }

		virtual bool Valid() const
		{ return _srcEnumerator->Valid(); }

		virtual DestType Get() const
		{ return _castPredicate(_srcEnumerator->Get()); }

		virtual void Next()
		{
			if (!Valid())
				return;

			do {
				_srcEnumerator->Next();
			} while (Valid() && _skipPredicate(_srcEnumerator->Get()));
		}

	private:
		void FindFirst()
		{
			while (Valid() && _skipPredicate(_srcEnumerator->Get()))
				_srcEnumerator->Next();
		}

		static bool NoSkip(ConstSrcTypeRef) { return false; }
		static DestType DefaultCast(ConstSrcTypeRef src) { return DestType(src); }
	};


	template<typename SrcType, typename CasterType>
	shared_ptr<IEnumerator<typename function_info<CasterType>::RetType> > WrapEnumerator(const shared_ptr<IEnumerator<SrcType> >& src, const CasterType& caster)
	{ return make_shared<EnumeratorWrapper<SrcType, typename function_info<CasterType>::RetType> >(src, caster); }

	template<typename SrcType, typename CasterType, typename SkipperType>
	shared_ptr<IEnumerator<typename function_info<CasterType>::RetType> > WrapEnumerator(const shared_ptr<IEnumerator<SrcType> >& src, const CasterType& caster, const SkipperType& skipper)
	{ return make_shared<EnumeratorWrapper<SrcType, typename function_info<CasterType>::RetType> >(src, caster, skipper); }


	namespace Detail
	{
		template < typename Signature >
		struct GetItemTypeFromItemDecl;

		template < typename T >
		struct GetItemTypeFromItemDecl< void (*)(T) >
		{ typedef T	ValueT; };


		template < typename SrcType, typename LifeAssurance >
		class EnumeratorCaster
		{
			typedef shared_ptr<IEnumerator<SrcType> >					SrcEnumeratorPtr;
			typedef typename GetConstReferenceType<SrcType>::ValueT		ConstSrcTypeRef;


			class Proxy : public EnumeratorWrapper<SrcType, SrcType>
			{
				typedef EnumeratorWrapper<SrcType, SrcType>	base;

			private:
				shared_ptr<LifeAssurance>	_assurance;

			public:
				Proxy(const SrcEnumeratorPtr& srcEnumerator, const shared_ptr<LifeAssurance>& assurance)
					: base(srcEnumerator), _assurance(assurance)
				{ }
			};

			template < typename DestType >
			class CastProxy : public EnumeratorWrapper<SrcType, DestType>
			{
				typedef EnumeratorWrapper<SrcType, DestType>	base;
			private:
				shared_ptr<LifeAssurance>	_assurance;

			public:
				CastProxy(const SrcEnumeratorPtr& srcEnumerator, const shared_ptr<LifeAssurance>& assurance)
					: base(srcEnumerator, &CastProxy::Cast, &CastProxy::Skip), _assurance(assurance)
				{ }

			private:
				static DestType Cast(ConstSrcTypeRef src)
				{ return dynamic_caster(src); }

				static bool Skip(ConstSrcTypeRef src)
				{ return !InstanceOf<typename GetSharedPtrParam<DestType>::ValueT>(src); }
			};

		private:
			SrcEnumeratorPtr			_srcEnumerator;
			shared_ptr<LifeAssurance>	_assurance; // In order to control the lifetime of the assurance if necessary

		public:
			EnumeratorCaster(const SrcEnumeratorPtr& srcEnumerator, const shared_ptr<LifeAssurance>& assurance)
				: _srcEnumerator(srcEnumerator), _assurance(assurance)
			{ }

			operator SrcEnumeratorPtr() const
			{ return make_shared<Proxy>(_srcEnumerator, _assurance); }

			template < typename DestType >
			operator shared_ptr<IEnumerator<DestType> > () const
			{ return make_shared<CastProxy<DestType> >(_srcEnumerator, _assurance); }
		};


		template < typename T, bool IsEnumerator_ = IsEnumerator<T>::Value >
		struct EnumeratorGetter
		{
			typedef EnumeratorCaster<typename T::ItemType, T>	EnumeratorPtrType;

			static EnumeratorPtrType Get(const shared_ptr<T>& obj)
			{ return EnumeratorPtrType(obj->GetEnumerator(), obj); }
		};

		template < typename T >
		struct EnumeratorGetter<T, true>
		{
			typedef EnumeratorCaster<typename T::ItemType, int>	EnumeratorPtrType;

			static EnumeratorPtrType Get(const shared_ptr<T>& obj)
			{ return EnumeratorPtrType(obj, null); }
		};

		template < typename ItemDeclFunc >
		struct ItemEnumeratorPtr : public shared_ptr<IEnumerator<typename stingray::Detail::GetItemTypeFromItemDecl<ItemDeclFunc>::ValueT> >
		{
			typedef shared_ptr<IEnumerator<typename stingray::Detail::GetItemTypeFromItemDecl<ItemDeclFunc>::ValueT> >	base;

			ItemEnumeratorPtr(const base& ptr) : base(ptr) { }
		};


		inline bool ForEach_ItemFilter(bool dummy, bool val) { return val; }
		inline bool ForEach_ItemFilter(bool dummy) { return true; }
	}

	template < typename T >
	typename Detail::EnumeratorGetter<T>::EnumeratorPtrType GetEnumeratorCaster(const shared_ptr<T>& obj)
	{ return Detail::EnumeratorGetter<T>::Get(obj); }


#define IN ,
#define WHERE ,
#define FOR_EACH__IMPL(ItemDecl_, SomethingToEnumerate_, ...) \
		for (bool __broken__ = false; !__broken__; __broken__ = true) \
			for (::stingray::Detail::ItemEnumeratorPtr<void(*)(ItemDecl_)> en(::stingray::GetEnumeratorCaster(SomethingToEnumerate_)); \
				 en && en->Valid() && !__broken__; \
				 en->Next()) \
				 for (bool __dummy_bool__ = true; __dummy_bool__ && !__broken__; ) \
					 for (ItemDecl_ = en->Get(); (__dummy_bool__ && ((__dummy_bool__ = false) == false) && ::stingray::Detail::ForEach_ItemFilter(true, ##__VA_ARGS__) && (__broken__ = true)); __broken__ = false)

	/** @} */

}


#endif
