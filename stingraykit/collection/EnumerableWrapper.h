#ifndef STINGRAYKIT_COLLECTION_ENUMERABLEWRAPPER_H
#define STINGRAYKIT_COLLECTION_ENUMERABLEWRAPPER_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/IEnumerable.h>
#include <stingraykit/function/function.h>
#include <stingraykit/optional.h>

namespace stingray
{

	template < typename SrcType, typename DestType >
	class EnumeratorWrapper : public IEnumerator<DestType>
	{
		typedef shared_ptr<IEnumerator<SrcType> >					SrcEnumeratorPtr;
		typedef typename AddConstLvalueReference<SrcType>::ValueT	ConstSrcTypeRef;
		typedef function<bool (ConstSrcTypeRef)>					FilterPredicate;
		typedef function<DestType (ConstSrcTypeRef)>				Caster;

	private:
		SrcEnumeratorPtr			_srcEnumerator;
		Caster						_caster;
		FilterPredicate				_filterPredicate;
		optional<SrcType>			_cache;

	public:
		EnumeratorWrapper(const SrcEnumeratorPtr& srcEnumerator)
			: _srcEnumerator(srcEnumerator), _caster(&EnumeratorWrapper::DefaultCast), _filterPredicate(&EnumeratorWrapper::NoFilter)
		{ FindFirst(); }

		EnumeratorWrapper(const SrcEnumeratorPtr& srcEnumerator, const Caster& caster)
			: _srcEnumerator(srcEnumerator), _caster(caster), _filterPredicate(&EnumeratorWrapper::NoFilter)
		{ FindFirst(); }

		EnumeratorWrapper(const SrcEnumeratorPtr& srcEnumerator, const Caster& caster, const FilterPredicate& filterPredicate)
			: _srcEnumerator(srcEnumerator), _caster(caster), _filterPredicate(filterPredicate)
		{ FindFirst(); }

		virtual bool Valid() const
		{ return _srcEnumerator->Valid(); }

		virtual DestType Get() const
		{ return _cache ? _caster(*_cache) : _caster(_srcEnumerator->Get()); }

		virtual void Next()
		{
			if (!Valid())
				return;

			_cache.reset();
			do {
				_srcEnumerator->Next();
				if (!Valid())
					break;

				const SrcType& cache = _srcEnumerator->Get();
				if (_filterPredicate(cache))
					_cache.emplace(cache);
			} while (!_cache);
		}

	private:
		void FindFirst()
		{
			while (Valid() && !_cache)
			{
				const SrcType& cache = _srcEnumerator->Get();
				if (_filterPredicate(cache))
					_cache.emplace(cache);
				else
					_srcEnumerator->Next();
			}
		}

		static DestType DefaultCast(ConstSrcTypeRef src)	{ return DestType(src); }
		static bool NoFilter(ConstSrcTypeRef)				{ return true; }
	};


	template < typename SrcEnumeratorType, typename CasterType >
	shared_ptr<IEnumerator<typename function_info<CasterType>::RetType> > WrapEnumerator(const shared_ptr<SrcEnumeratorType>& src, const CasterType& caster)
	{ return make_shared_ptr<EnumeratorWrapper<typename SrcEnumeratorType::ItemType, typename function_info<CasterType>::RetType> >(src, caster); }


	template < typename SrcEnumeratorType, typename CasterType, typename FilterPredicate >
	shared_ptr<IEnumerator<typename function_info<CasterType>::RetType> > WrapEnumerator(const shared_ptr<SrcEnumeratorType>& src, const CasterType& caster, const FilterPredicate& filterPredicate)
	{ return make_shared_ptr<EnumeratorWrapper<typename SrcEnumeratorType::ItemType, typename function_info<CasterType>::RetType> >(src, caster, filterPredicate); }


	template < typename SrcType, typename DestType >
	class EnumerableWrapper : public virtual IEnumerable<DestType>
	{
		typedef shared_ptr<IEnumerable<SrcType> >					SrcEnumerablePtr;
		typedef typename AddConstLvalueReference<SrcType>::ValueT	ConstSrcTypeRef;
		typedef function<bool (ConstSrcTypeRef)>					FilterPredicate;
		typedef function<DestType (ConstSrcTypeRef)>				Caster;

	private:
		SrcEnumerablePtr			_srcEnumerable;
		Caster						_caster;
		FilterPredicate				_filterPredicate;

	public:
		EnumerableWrapper(const SrcEnumerablePtr& srcEnumerable)
			: _srcEnumerable(srcEnumerable), _caster(&EnumerableWrapper::DefaultCast), _filterPredicate(&EnumerableWrapper::NoFilter)
		{ }

		EnumerableWrapper(const SrcEnumerablePtr& srcEnumerable, const Caster& caster)
			: _srcEnumerable(srcEnumerable), _caster(caster), _filterPredicate(&EnumerableWrapper::NoFilter)
		{ }

		EnumerableWrapper(const SrcEnumerablePtr& srcEnumerable, const Caster& caster, const FilterPredicate& filterPredicate)
			: _srcEnumerable(srcEnumerable), _caster(caster), _filterPredicate(filterPredicate)
		{ }

		virtual shared_ptr<IEnumerator<DestType> > GetEnumerator() const
		{ return WrapEnumerator(_srcEnumerable->GetEnumerator(), _caster, _filterPredicate); }

	private:
		static DestType DefaultCast(ConstSrcTypeRef src)	{ return DestType(src); }
		static bool NoFilter(ConstSrcTypeRef)				{ return true; }
	};


	template < typename SrcEnumerableType, typename CasterType >
	shared_ptr<IEnumerable<typename function_info<CasterType>::RetType> > WrapEnumerable(const shared_ptr<SrcEnumerableType>& src, const CasterType& caster)
	{ return make_shared_ptr<EnumerableWrapper<typename SrcEnumerableType::ItemType, typename function_info<CasterType>::RetType> >(src, caster); }


	template < typename SrcEnumerableType, typename CasterType, typename FilterPredicate >
	shared_ptr<IEnumerable<typename function_info<CasterType>::RetType> > WrapEnumerable(const shared_ptr<SrcEnumerableType>& src, const CasterType& caster, const FilterPredicate& filterPredicate)
	{ return make_shared_ptr<EnumerableWrapper<typename SrcEnumerableType::ItemType, typename function_info<CasterType>::RetType> >(src, caster, filterPredicate); }


	namespace Detail
	{
		template < typename SrcType >
		class EnumerableCaster
		{
			typedef shared_ptr<IEnumerable<SrcType> >					SrcEnumerablePtr;

			template < typename DestType >
			class CastProxy : public EnumerableWrapper<SrcType, DestType>
			{
				typedef EnumerableWrapper<SrcType, DestType>				base;
				typedef typename AddConstLvalueReference<SrcType>::ValueT	ConstSrcTypeRef;

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
			{ return make_shared_ptr<CastProxy<DestType> >(_srcEnumerable); }
		};
	}


	template < typename T >
	typename Detail::EnumerableCaster<typename T::ItemType> GetEnumerableCaster(const shared_ptr<T>& enumerable)
	{ return Detail::EnumerableCaster<typename T::ItemType>(enumerable); }

}

#endif
