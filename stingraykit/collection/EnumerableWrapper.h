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

	namespace Detail
	{
		template < typename SrcType, typename DestType >
		class EnumeratorWrapper : public IEnumerator<DestType>
		{
			using SrcEnumeratorPtr = shared_ptr<IEnumerator<SrcType>>;
			using ConstSrcTypeRef = typename AddConstLvalueReference<SrcType>::ValueT;
			using FilterPredicate = function<bool (ConstSrcTypeRef)>;
			using Caster = function<DestType (ConstSrcTypeRef)>;

		private:
			SrcEnumeratorPtr			_srcEnumerator;
			Caster						_caster;
			FilterPredicate				_filterPredicate;
			optional<SrcType>			_cache;

		public:
			EnumeratorWrapper(const SrcEnumeratorPtr& srcEnumerator)
				: _srcEnumerator(STINGRAYKIT_REQUIRE_NOT_NULL(srcEnumerator)), _caster(&EnumeratorWrapper::DefaultCast), _filterPredicate(&EnumeratorWrapper::NoFilter)
			{ FindNext(); }

			EnumeratorWrapper(const SrcEnumeratorPtr& srcEnumerator, const Caster& caster)
				: _srcEnumerator(STINGRAYKIT_REQUIRE_NOT_NULL(srcEnumerator)), _caster(caster), _filterPredicate(&EnumeratorWrapper::NoFilter)
			{ FindNext(); }

			EnumeratorWrapper(const SrcEnumeratorPtr& srcEnumerator, const Caster& caster, const FilterPredicate& filterPredicate)
				: _srcEnumerator(STINGRAYKIT_REQUIRE_NOT_NULL(srcEnumerator)), _caster(caster), _filterPredicate(filterPredicate)
			{ FindNext(); }

			bool Valid() const override
			{ return _cache.is_initialized(); }

			DestType Get() const override
			{
				STINGRAYKIT_CHECK(_cache, "Enumerator is not valid!");
				return _caster(*_cache);
			}

			void Next() override
			{
				STINGRAYKIT_CHECK(_cache, "Enumerator is not valid!");

				_cache.reset();
				_srcEnumerator->Next();

				FindNext();
			}

		private:
			void FindNext()
			{
				while (!_cache && _srcEnumerator->Valid())
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

		template < typename SrcType >
		class EnumeratorWrapperProxy
		{
			using SrcEnumeratorPtr = shared_ptr<IEnumerator<SrcType>>;

		private:
			SrcEnumeratorPtr			_srcEnumerator;

		public:
			explicit EnumeratorWrapperProxy(const SrcEnumeratorPtr& srcEnumerator)
				: _srcEnumerator(srcEnumerator)
			{ }

			template < typename DestType >
			operator shared_ptr<IEnumerator<DestType>>() const
			{ return make_shared_ptr<EnumeratorWrapper<SrcType, DestType>>(_srcEnumerator); }
		};
	}


	template < typename SrcEnumeratorType >
	Detail::EnumeratorWrapperProxy<typename SrcEnumeratorType::ItemType> WrapEnumerator(const shared_ptr<SrcEnumeratorType>& src)
	{ return Detail::EnumeratorWrapperProxy<typename SrcEnumeratorType::ItemType>(src); }


	template < typename SrcEnumeratorType, typename CasterType >
	shared_ptr<IEnumerator<typename function_info<CasterType>::RetType>> WrapEnumerator(const shared_ptr<SrcEnumeratorType>& src, const CasterType& caster)
	{ return make_shared_ptr<Detail::EnumeratorWrapper<typename SrcEnumeratorType::ItemType, typename function_info<CasterType>::RetType>>(src, caster); }


	template < typename SrcEnumeratorType, typename CasterType, typename FilterPredicate >
	shared_ptr<IEnumerator<typename function_info<CasterType>::RetType>> WrapEnumerator(const shared_ptr<SrcEnumeratorType>& src, const CasterType& caster, const FilterPredicate& filterPredicate)
	{ return make_shared_ptr<Detail::EnumeratorWrapper<typename SrcEnumeratorType::ItemType, typename function_info<CasterType>::RetType>>(src, caster, filterPredicate); }

	namespace Detail
	{
		template < typename SrcType, typename DestType >
		class EnumerableWrapper : public virtual IEnumerable<DestType>
		{
			using SrcEnumerablePtr = shared_ptr<IEnumerable<SrcType>>;
			using ConstSrcTypeRef = typename AddConstLvalueReference<SrcType>::ValueT;
			using FilterPredicate = function<bool (ConstSrcTypeRef)>;
			using Caster = function<DestType (ConstSrcTypeRef)>;

		private:
			SrcEnumerablePtr			_srcEnumerable;
			Caster						_caster;
			FilterPredicate				_filterPredicate;

		public:
			EnumerableWrapper(const SrcEnumerablePtr& srcEnumerable)
				: _srcEnumerable(STINGRAYKIT_REQUIRE_NOT_NULL(srcEnumerable)), _caster(&EnumerableWrapper::DefaultCast), _filterPredicate(&EnumerableWrapper::NoFilter)
			{ }

			EnumerableWrapper(const SrcEnumerablePtr& srcEnumerable, const Caster& caster)
				: _srcEnumerable(STINGRAYKIT_REQUIRE_NOT_NULL(srcEnumerable)), _caster(caster), _filterPredicate(&EnumerableWrapper::NoFilter)
			{ }

			EnumerableWrapper(const SrcEnumerablePtr& srcEnumerable, const Caster& caster, const FilterPredicate& filterPredicate)
				: _srcEnumerable(STINGRAYKIT_REQUIRE_NOT_NULL(srcEnumerable)), _caster(caster), _filterPredicate(filterPredicate)
			{ }

			shared_ptr<IEnumerator<DestType>> GetEnumerator() const override
			{ return WrapEnumerator(_srcEnumerable->GetEnumerator(), _caster, _filterPredicate); }

		private:
			static DestType DefaultCast(ConstSrcTypeRef src)	{ return DestType(src); }
			static bool NoFilter(ConstSrcTypeRef)				{ return true; }
		};

		template < typename SrcType >
		class EnumerableWrapperProxy
		{
			using SrcEnumerablePtr = shared_ptr<IEnumerable<SrcType>>;

		private:
			SrcEnumerablePtr			_srcEnumerable;

		public:
			explicit EnumerableWrapperProxy(const SrcEnumerablePtr& srcEnumerable)
				: _srcEnumerable(srcEnumerable)
			{ }

			template < typename DestType >
			operator shared_ptr<IEnumerable<DestType>>() const
			{ return make_shared_ptr<EnumerableWrapper<SrcType, DestType>>(_srcEnumerable); }
		};
	}


	template < typename SrcEnumerableType >
	Detail::EnumerableWrapperProxy<typename SrcEnumerableType::ItemType> WrapEnumerable(const shared_ptr<SrcEnumerableType>& src)
	{ return Detail::EnumerableWrapperProxy<typename SrcEnumerableType::ItemType>(src); }


	template < typename SrcEnumerableType, typename CasterType >
	shared_ptr<IEnumerable<typename function_info<CasterType>::RetType>> WrapEnumerable(const shared_ptr<SrcEnumerableType>& src, const CasterType& caster)
	{ return make_shared_ptr<Detail::EnumerableWrapper<typename SrcEnumerableType::ItemType, typename function_info<CasterType>::RetType>>(src, caster); }


	template < typename SrcEnumerableType, typename CasterType, typename FilterPredicate >
	shared_ptr<IEnumerable<typename function_info<CasterType>::RetType>> WrapEnumerable(const shared_ptr<SrcEnumerableType>& src, const CasterType& caster, const FilterPredicate& filterPredicate)
	{ return make_shared_ptr<Detail::EnumerableWrapper<typename SrcEnumerableType::ItemType, typename function_info<CasterType>::RetType>>(src, caster, filterPredicate); }


	namespace Detail
	{
		template < typename SrcType >
		class EnumeratorCaster
		{
			using SrcEnumeratorPtr = shared_ptr<IEnumerator<SrcType>>;
			using ConstSrcTypeRef = typename AddConstLvalueReference<SrcType>::ValueT;

		private:
			SrcEnumeratorPtr			_srcEnumerator;

		public:
			EnumeratorCaster(const SrcEnumeratorPtr& srcEnumerator) : _srcEnumerator(STINGRAYKIT_REQUIRE_NOT_NULL(srcEnumerator))
			{ }

			operator SrcEnumeratorPtr () const
			{ return _srcEnumerator; }

			template < typename DestType >
			operator shared_ptr<IEnumerator<DestType>> () const
			{ return WrapEnumerator(_srcEnumerator, &EnumeratorCaster::Cast<DestType>, InstanceOfPredicate<typename GetSharedPtrParam<DestType>::ValueT>()); }

		private:
			template < typename DestType >
			static DestType Cast(ConstSrcTypeRef src)
			{ return dynamic_caster(src); }
		};
	}


	template < typename SrcEnumeratorType >
	Detail::EnumeratorCaster<typename SrcEnumeratorType::ItemType> GetEnumeratorCaster(const shared_ptr<SrcEnumeratorType>& enumerator)
	{ return Detail::EnumeratorCaster<typename SrcEnumeratorType::ItemType>(enumerator); }


	namespace Detail
	{
		template < typename SrcType >
		class EnumerableCaster
		{
			using SrcEnumerablePtr = shared_ptr<IEnumerable<SrcType>>;
			using ConstSrcTypeRef = typename AddConstLvalueReference<SrcType>::ValueT;

		private:
			SrcEnumerablePtr			_srcEnumerable;

		public:
			EnumerableCaster(const SrcEnumerablePtr& srcEnumerable) : _srcEnumerable(STINGRAYKIT_REQUIRE_NOT_NULL(srcEnumerable))
			{ }

			operator SrcEnumerablePtr () const
			{ return _srcEnumerable; }

			template < typename DestType >
			operator shared_ptr<IEnumerable<DestType>> () const
			{ return WrapEnumerable(_srcEnumerable, &EnumerableCaster::Cast<DestType>, InstanceOfPredicate<typename GetSharedPtrParam<DestType>::ValueT>()); }

		private:
			template < typename DestType >
			static DestType Cast(ConstSrcTypeRef src)
			{ return dynamic_caster(src); }
		};
	}


	template < typename SrcEnumerableType >
	Detail::EnumerableCaster<typename SrcEnumerableType::ItemType> GetEnumerableCaster(const shared_ptr<SrcEnumerableType>& enumerable)
	{ return Detail::EnumerableCaster<typename SrcEnumerableType::ItemType>(enumerable); }

}

#endif
