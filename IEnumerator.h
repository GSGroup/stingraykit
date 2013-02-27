#ifndef STINGRAY_TOOLKIT_IENUMERATOR_H
#define STINGRAY_TOOLKIT_IENUMERATOR_H


#include <stingray/toolkit/shared_ptr.h>
#include <stingray/toolkit/function.h>
#include <stingray/toolkit/dynamic_caster.h>
#include <stingray/toolkit/MetaProgramming.h>


#define TOOLKIT_DECLARE_ENUMERATOR(ClassName) \
		typedef stingray::IEnumerator<ClassName>				ClassName##Enumerator; \
		TOOLKIT_DECLARE_PTR(ClassName##Enumerator)

namespace stingray
{


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


	template < typename T >
	struct EmptyEnumerator : public virtual IEnumerator<T>
	{
		virtual bool Valid() const	{ return false; }
		virtual T Get() const		{ TOOLKIT_THROW(NotSupportedException()); }
		virtual void Next()			{ TOOLKIT_THROW(NotSupportedException()); }
	};


	class EmptyEnumeratorProxy
	{
	public:
		template< typename U >
		operator shared_ptr<IEnumerator<U> >() const
		{ return make_shared<EmptyEnumerator<U> >(); }
	};


	inline EmptyEnumeratorProxy MakeEmptyEnumerator()
	{ return EmptyEnumeratorProxy(); }


	template < typename T >
	struct OneItemEnumerator : public virtual IEnumerator<T>
	{
	private:
		bool	_valid;
		T		_value;

	public:
		OneItemEnumerator(typename GetConstReferenceType<T>::ValueT value)
			: _valid(true), _value(value)
		{ }

		virtual bool Valid() const			{ return _valid; }
		virtual void Next()					{ _valid = false; }

		virtual T Get() const
		{
			if (!_valid)
				TOOLKIT_THROW(std::runtime_error("Invalid enumerator!"));
			return _value;
		}
	};


	template< typename T >
	class OneItemEnumeratorProxy
	{
	private:
		T	_item;

	public:
		explicit OneItemEnumeratorProxy(const T& item)
			: _item(item)
		{ }

		template< typename U >
		operator shared_ptr<IEnumerator<U> >() const
		{ return make_shared<OneItemEnumerator<U> >(_item); }
	};


	template< typename T >
	OneItemEnumeratorProxy<T> MakeOneItemEnumerator(const T& item)
	{ return OneItemEnumeratorProxy<T>(item); }


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

	/*! \cond GS_INTERNAL */

	namespace Detail
	{
		template<typename EnumeratedT>
		class JoiningEnumerator : public virtual IEnumerator<EnumeratedT>
		{
			typedef shared_ptr<IEnumerator<EnumeratedT> > TargetEnumeratorPtr;

			TargetEnumeratorPtr _first, _second;

		public:
			JoiningEnumerator(const TargetEnumeratorPtr& first, const TargetEnumeratorPtr& second) :
				_first(TOOLKIT_REQUIRE_NOT_NULL(first)),
				_second(TOOLKIT_REQUIRE_NOT_NULL(second))
			{}

			virtual bool Valid() const
			{ return _first->Valid() || _second->Valid(); }

			virtual EnumeratedT Get() const
			{ return _first->Valid() ? _first->Get() : _second->Get(); }

			virtual void Next()
			{
				if (_first->Valid())
					_first->Next();
				else
					_second->Next();
			}
		};
	}

	template<typename EnumeratedT>
	shared_ptr<IEnumerator<EnumeratedT> > JoinEnumerators(const shared_ptr<IEnumerator<EnumeratedT> >& first, const shared_ptr<IEnumerator<EnumeratedT> >& second)
	{ return make_shared<Detail::JoiningEnumerator<EnumeratedT> >(first, second); }

	template<typename EnumeratedT>
	class EnumeratorJoiner
	{
		typedef shared_ptr<IEnumerator<EnumeratedT> >	EnumeratorPtr;

	private:
		EnumeratorPtr	_result;

	public:
		operator EnumeratorPtr () const { return _result; }
		EnumeratorJoiner& operator % (const EnumeratorPtr& e)
		{
			if (_result)
				_result = JoinEnumerators(_result, e);
			else
				_result = e;
			return *this;
		}
	};

	namespace Detail
	{
		template < typename Signature >
		struct GetItemTypeFromItemDecl;

		template < typename T >
		struct GetItemTypeFromItemDecl< void (*)(T) >
		{ typedef T	ValueT; };


		template < typename SrcType, typename CollectionType >
		class EnumeratorCaster
		{
			typedef shared_ptr<IEnumerator<SrcType> >					SrcEnumeratorPtr;
			typedef typename GetConstReferenceType<SrcType>::ValueT		ConstSrcTypeRef;


			class Proxy : public EnumeratorWrapper<SrcType, SrcType>
			{
				typedef EnumeratorWrapper<SrcType, SrcType>	base;

			private:
				shared_ptr<CollectionType>	_collection;

			public:
				Proxy(const SrcEnumeratorPtr& srcEnumerator, const shared_ptr<CollectionType>& collection)
					: base(srcEnumerator), _collection(collection)
				{ }
			};

			template < typename DestType >
			class CastProxy : public EnumeratorWrapper<SrcType, DestType>
			{
				typedef EnumeratorWrapper<SrcType, DestType>	base;
			private:
				shared_ptr<CollectionType>	_collection;

			public:
				CastProxy(const SrcEnumeratorPtr& srcEnumerator, const shared_ptr<CollectionType>& collection)
					: base(srcEnumerator, &CastProxy::Cast, &CastProxy::Skip), _collection(collection)
				{ }

			private:
				static DestType Cast(ConstSrcTypeRef src)
				{ return dynamic_caster(src); }

				static bool Skip(ConstSrcTypeRef src)
				{ return !InstanceOf<typename GetSharedPtrParam<DestType>::ValueT>(src); }
			};

		private:
			SrcEnumeratorPtr			_srcEnumerator;
			shared_ptr<CollectionType>	_collection; // In order to control the lifetime of the collection if necessary

		public:
			EnumeratorCaster(const SrcEnumeratorPtr& srcEnumerator, const shared_ptr<CollectionType>& collection)
				: _srcEnumerator(srcEnumerator), _collection(collection)
			{ }

			operator SrcEnumeratorPtr() const
			{ return make_shared<Proxy>(_srcEnumerator, _collection); }

			template < typename DestType >
			operator shared_ptr<IEnumerator<DestType> > () const
			{ return make_shared<CastProxy<DestType> >(_srcEnumerator, _collection); }
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

#define FOR_EACH(...) FOR_EACH__IMPL(__VA_ARGS__)

// --- Usage:
// FOR_EACH(ISomeObjectPtr obj IN something->GetObjectCollection())
// --- or
// FOR_EACH(ISomeObjectPtr obj IN something->GetObjectCollection() WHERE obj->GetProperty() != 1)

	/*! \endcond */


}


#endif
