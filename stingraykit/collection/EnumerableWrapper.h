#ifndef STINGRAYKIT_COLLECTION_ENUMERABLEWRAPPER_H
#define STINGRAYKIT_COLLECTION_ENUMERABLEWRAPPER_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/IEnumerable.h>
#include <stingraykit/optional.h>

namespace stingray
{

	namespace Detail
	{
		template < typename DestType >
		struct DefaultEnumeratorCaster : public function_info<DestType, UnspecifiedParamTypes>
		{
			template < typename SrcType >
			DestType operator () (const SrcType& src) const
			{ return DestType(src); }
		};

		struct DefaultEnumeratorFilter : public function_info<bool, UnspecifiedParamTypes>
		{
			template < typename SrcType >
			bool operator () (const SrcType&) const
			{ return true; }
		};

		template < typename SrcType, typename CasterFunc, typename FilterPredicate >
		class EnumeratorWrapper : public virtual IEnumerator<typename function_info<CasterFunc>::RetType>
		{
			using base = IEnumerator<typename function_info<CasterFunc>::RetType>;
			using SrcEnumeratorPtr = shared_ptr<IEnumerator<SrcType>>;

		private:
			SrcEnumeratorPtr			_srcEnumerator;
			CasterFunc					_caster;
			FilterPredicate				_filter;
			optional<SrcType>			_cache;

		public:
			EnumeratorWrapper(const SrcEnumeratorPtr& srcEnumerator, const CasterFunc& caster, const FilterPredicate& filter)
				: _srcEnumerator(STINGRAYKIT_REQUIRE_NOT_NULL(srcEnumerator)), _caster(caster), _filter(filter)
			{ FindNext(); }

			bool Valid() const override
			{ return _cache.is_initialized(); }

			typename base::ItemType Get() const override
			{
				STINGRAYKIT_CHECK(_cache, "Enumerator is not valid!");
				return FunctorInvoker::InvokeArgs(_caster, *_cache);
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
					const SrcType cache = _srcEnumerator->Get();
					if (FunctorInvoker::InvokeArgs(_filter, cache))
						_cache.emplace(cache);
					else
						_srcEnumerator->Next();
				}
			}
		};

		template < typename SrcType >
		class EnumeratorWrapperProxy
		{
			using SrcEnumeratorPtr = shared_ptr<IEnumerator<SrcType>>;

		private:
			SrcEnumeratorPtr			_srcEnumerator;

		public:
			explicit EnumeratorWrapperProxy(const SrcEnumeratorPtr& srcEnumerator)
				: _srcEnumerator(STINGRAYKIT_REQUIRE_NOT_NULL(srcEnumerator))
			{ }

			template < typename DestType >
			operator shared_ptr<IEnumerator<DestType>>() const
			{ return make_shared_ptr<EnumeratorWrapper<SrcType, DefaultEnumeratorCaster<DestType>, DefaultEnumeratorFilter>>(_srcEnumerator, DefaultEnumeratorCaster<DestType>(), DefaultEnumeratorFilter()); }
		};
	}


	template < typename SrcEnumeratorType >
	Detail::EnumeratorWrapperProxy<typename SrcEnumeratorType::ItemType> WrapEnumerator(const shared_ptr<SrcEnumeratorType>& src)
	{ return Detail::EnumeratorWrapperProxy<typename SrcEnumeratorType::ItemType>(src); }


	template < typename SrcEnumeratorType, typename CasterFunc >
	shared_ptr<IEnumerator<typename function_info<CasterFunc>::RetType>> WrapEnumerator(const shared_ptr<SrcEnumeratorType>& src, const CasterFunc& caster)
	{ return make_shared_ptr<Detail::EnumeratorWrapper<typename SrcEnumeratorType::ItemType, CasterFunc, Detail::DefaultEnumeratorFilter>>(src, caster, Detail::DefaultEnumeratorFilter()); }


	template < typename SrcEnumeratorType, typename CasterFunc, typename FilterPredicate >
	shared_ptr<IEnumerator<typename function_info<CasterFunc>::RetType>> WrapEnumerator(const shared_ptr<SrcEnumeratorType>& src, const CasterFunc& caster, const FilterPredicate& filter)
	{ return make_shared_ptr<Detail::EnumeratorWrapper<typename SrcEnumeratorType::ItemType, CasterFunc, FilterPredicate>>(src, caster, filter); }


	namespace Detail
	{
		template < typename SrcType, typename CasterFunc, typename FilterPredicate >
		class EnumerableWrapper : public virtual IEnumerable<typename function_info<CasterFunc>::RetType>
		{
			using base = IEnumerable<typename function_info<CasterFunc>::RetType>;
			using SrcEnumerablePtr = shared_ptr<IEnumerable<SrcType>>;

		private:
			SrcEnumerablePtr			_srcEnumerable;
			CasterFunc					_caster;
			FilterPredicate				_filter;

		public:
			EnumerableWrapper(const SrcEnumerablePtr& srcEnumerable, const CasterFunc& caster, const FilterPredicate& filter)
				: _srcEnumerable(STINGRAYKIT_REQUIRE_NOT_NULL(srcEnumerable)), _caster(caster), _filter(filter)
			{ }

			shared_ptr<IEnumerator<typename base::ItemType>> GetEnumerator() const override
			{ return WrapEnumerator(_srcEnumerable->GetEnumerator(), _caster, _filter); }
		};

		template < typename SrcType >
		class EnumerableWrapperProxy
		{
			using SrcEnumerablePtr = shared_ptr<IEnumerable<SrcType>>;

		private:
			SrcEnumerablePtr			_srcEnumerable;

		public:
			explicit EnumerableWrapperProxy(const SrcEnumerablePtr& srcEnumerable)
				: _srcEnumerable(STINGRAYKIT_REQUIRE_NOT_NULL(srcEnumerable))
			{ }

			template < typename DestType >
			operator shared_ptr<IEnumerable<DestType>>() const
			{ return make_shared_ptr<EnumerableWrapper<SrcType, DefaultEnumeratorCaster<DestType>, DefaultEnumeratorFilter>>(_srcEnumerable, DefaultEnumeratorCaster<DestType>(), DefaultEnumeratorFilter()); }
		};
	}


	template < typename SrcEnumerableType >
	Detail::EnumerableWrapperProxy<typename SrcEnumerableType::ItemType> WrapEnumerable(const shared_ptr<SrcEnumerableType>& src)
	{ return Detail::EnumerableWrapperProxy<typename SrcEnumerableType::ItemType>(src); }


	template < typename SrcEnumerableType, typename CasterFunc >
	shared_ptr<IEnumerable<typename function_info<CasterFunc>::RetType>> WrapEnumerable(const shared_ptr<SrcEnumerableType>& src, const CasterFunc& caster)
	{ return make_shared_ptr<Detail::EnumerableWrapper<typename SrcEnumerableType::ItemType, CasterFunc, Detail::DefaultEnumeratorFilter>>(src, caster, Detail::DefaultEnumeratorFilter()); }


	template < typename SrcEnumerableType, typename CasterFunc, typename FilterPredicate >
	shared_ptr<IEnumerable<typename function_info<CasterFunc>::RetType>> WrapEnumerable(const shared_ptr<SrcEnumerableType>& src, const CasterFunc& caster, const FilterPredicate& filter)
	{ return make_shared_ptr<Detail::EnumerableWrapper<typename SrcEnumerableType::ItemType, CasterFunc, FilterPredicate>>(src, caster, filter); }

}

#endif
