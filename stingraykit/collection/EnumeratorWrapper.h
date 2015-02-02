#ifndef STINGRAY_STINGRAYKIT_STINGRAYKIT_COLLECTION_ENUMERATORWRAPPER_H
#define STINGRAY_STINGRAYKIT_STINGRAYKIT_COLLECTION_ENUMERATORWRAPPER_H


#include <stingraykit/collection/IEnumerator.h>

namespace stingray
{

	template < typename SrcType, typename DestType >
	class EnumeratorWrapper : public IEnumerator<DestType>
	{
		typedef shared_ptr<IEnumerator<SrcType> >					SrcEnumeratorPtr;
		typedef typename GetConstReferenceType<SrcType>::ValueT		ConstSrcTypeRef;
		typedef function< bool(ConstSrcTypeRef) >					FilterPredicate;
		typedef function< DestType (ConstSrcTypeRef) >				Caster;

	private:
		SrcEnumeratorPtr		_srcEnumerator;
		Caster					_caster;
		FilterPredicate			_filterPredicate;

	public:
		EnumeratorWrapper(const SrcEnumeratorPtr& srcEnumerator) :
			_srcEnumerator(srcEnumerator), _caster(&EnumeratorWrapper::DefaultCast), _filterPredicate(&EnumeratorWrapper::NoFilter)
		{ FindFirst(); }

		EnumeratorWrapper(const SrcEnumeratorPtr& srcEnumerator, const Caster& caster) :
			_srcEnumerator(srcEnumerator), _caster(caster), _filterPredicate(&EnumeratorWrapper::NoFilter)
		{ FindFirst(); }

		EnumeratorWrapper(const SrcEnumeratorPtr& srcEnumerator, const Caster& caster, const FilterPredicate& filterPredicate) :
			_srcEnumerator(srcEnumerator), _caster(caster), _filterPredicate(filterPredicate)
		{ FindFirst(); }

		virtual bool Valid() const
		{ return _srcEnumerator->Valid(); }

		virtual DestType Get() const
		{ return _caster(_srcEnumerator->Get()); }

		virtual void Next()
		{
			if (!Valid())
				return;

			do {
				_srcEnumerator->Next();
			} while (Valid() && !_filterPredicate(_srcEnumerator->Get()));
		}

	private:
		void FindFirst()
		{
			while (Valid() && !_filterPredicate(_srcEnumerator->Get()))
				_srcEnumerator->Next();
		}

		static DestType DefaultCast(ConstSrcTypeRef src)	{ return DestType(src); }
		static bool NoFilter(ConstSrcTypeRef)				{ return true; }
	};


	template<typename SrcType, typename CasterType>
	shared_ptr<IEnumerator<typename function_info<CasterType>::RetType> > WrapEnumerator(const shared_ptr<IEnumerator<SrcType> >& src, const CasterType& caster)
	{ return make_shared<EnumeratorWrapper<SrcType, typename function_info<CasterType>::RetType> >(src, caster); }


	template<typename SrcType, typename CasterType, typename FilterPredicate>
	shared_ptr<IEnumerator<typename function_info<CasterType>::RetType> > WrapEnumerator(const shared_ptr<IEnumerator<SrcType> >& src, const CasterType& caster, const FilterPredicate& filterPredicate)
	{ return make_shared<EnumeratorWrapper<SrcType, typename function_info<CasterType>::RetType> >(src, caster, filterPredicate); }

}

#endif
