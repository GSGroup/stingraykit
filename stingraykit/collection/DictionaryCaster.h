#ifndef STINGRAYKIT_COLLECTION_DICTIONARYCASTER_H
#define STINGRAYKIT_COLLECTION_DICTIONARYCASTER_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/ForEach.h>
#include <stingraykit/collection/IObservableDictionary.h>
#include <stingraykit/signal/signals.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename KeyType, typename SrcType, typename DestType, typename SrcDictionaryType = IReadonlyDictionary<KeyType, SrcType> >
	class DictionaryCaster : public virtual IReadonlyDictionary<KeyType, DestType>
	{
		typedef IReadonlyDictionary<KeyType, DestType> base;
		typedef DictionaryCaster<KeyType, SrcType, DestType, SrcDictionaryType> Self;

		STINGRAYKIT_DECLARE_PTR(SrcDictionaryType);

	public:
		typedef function<DestType (const SrcType&)> Caster;
		typedef function<bool (const DestType&, SrcType&)> BackCaster;

	protected:
		typedef typename SrcDictionaryType::PairType SrcPairType;
		typedef typename base::PairType DestPairType;

	protected:
		const SrcDictionaryTypePtr							_wrapped;
		const Caster										_caster;
		const BackCaster									_backCaster;

	public:
		DictionaryCaster(const SrcDictionaryTypePtr& wrapped, const Caster& caster = &Self::DefaultCast, const BackCaster& backCaster = &Self::DefaultBackCast)
			: _wrapped(STINGRAYKIT_REQUIRE_NOT_NULL(wrapped)), _caster(caster), _backCaster(backCaster)
		{ }

		virtual shared_ptr<IEnumerator<DestPairType> > GetEnumerator() const
		{ return WrapEnumerator(_wrapped->GetEnumerator(), Bind(&Self::PairCast, _caster, _1)); }

		virtual shared_ptr<IEnumerable<DestPairType> > Reverse() const
		{ return WrapEnumerable(_wrapped->Reverse(), Bind(&Self::PairCast, _caster, _1)); }

		virtual size_t GetCount() const
		{ return _wrapped->GetCount(); }

		virtual bool IsEmpty() const
		{ return _wrapped->IsEmpty(); }

		virtual bool ContainsKey(const KeyType& key) const
		{ return _wrapped->ContainsKey(key); }

		virtual shared_ptr<IEnumerator<DestPairType> > Find(const KeyType& key) const
		{ return WrapEnumerator(_wrapped->Find(key), Bind(&Self::PairCast, _caster, _1)); }

		virtual shared_ptr<IEnumerator<DestPairType> > ReverseFind(const KeyType& key) const
		{ return WrapEnumerator(_wrapped->ReverseFind(key), Bind(&Self::PairCast, _caster, _1)); }

		virtual DestType Get(const KeyType& key) const
		{ return _caster(_wrapped->Get(key)); }

		virtual bool TryGet(const KeyType& key, DestType& value) const
		{
			SrcType value_;
			if (_wrapped->TryGet(key, value_))
			{
				value = _caster(value_);
				return true;
			}
			return false;
		}

		static typename EnableIf<IsConvertible<SrcType, DestType>::Value, DestType>::ValueT DefaultCast(const SrcType& value)
		{ return DestType(value); }

		static typename EnableIf<IsSharedPtr<SrcType>::Value && IsSharedPtr<DestType>::Value, bool>::ValueT DefaultBackCast(const DestType& value, SrcType& result)
		{
			result = dynamic_caster(value);
			return result.is_initialized();
		}

	private:
		static DestPairType PairCast(const Caster& caster, const SrcPairType& pair)
		{ return DestPairType(pair.Key, caster(pair.Value)); }
	};


	template < typename KeyType, typename SrcType, typename DestType >
	class ObservableDictionaryCaster : public DictionaryCaster<KeyType, SrcType, DestType, IReadonlyObservableDictionary<KeyType, SrcType> >, public virtual IReadonlyObservableDictionary<KeyType, DestType>
	{
		typedef DictionaryCaster<KeyType, SrcType, DestType, IReadonlyObservableDictionary<KeyType, SrcType> > base;
		typedef ObservableDictionaryCaster<KeyType, SrcType, DestType> Self;

		typedef IReadonlyObservableDictionary<KeyType, SrcType> SrcDictionaryType;
		STINGRAYKIT_DECLARE_PTR(SrcDictionaryType);

		typedef signal_policies::threading::ExternalMutexPointer ExternalMutexPointer;

	public:
		typedef typename IReadonlyObservableDictionary<KeyType, DestType>::OnChangedSignature OnChangedSignature;

		typedef typename base::DestPairType DestPairType;

		typedef typename base::Caster Caster;
		typedef typename base::BackCaster BackCaster;

	private:
		signal<OnChangedSignature, ExternalMutexPointer>	_onChanged;
		const Token											_connection;

	public:
		explicit ObservableDictionaryCaster(const SrcDictionaryTypePtr& wrapped, const Caster& caster = &base::DefaultCast, const BackCaster& backCaster = &base::DefaultBackCast)
			:	base(wrapped, caster, backCaster),
				_onChanged(ExternalMutexPointer(shared_ptr<const Mutex>(base::_wrapped, &base::_wrapped->GetSyncRoot())), Bind(&Self::OnChangedPopulator, this, _1)),
				_connection(base::_wrapped->OnChanged().connect(Bind(&Self::ChangedHandler, this, _1, _2, _3)))
		{ }

		virtual signal_connector<OnChangedSignature> OnChanged() const
		{ return _onChanged.connector(); }

		virtual const Mutex& GetSyncRoot() const
		{ return base::_wrapped->GetSyncRoot(); }

	private:
		void OnChangedPopulator(const function<OnChangedSignature>& slot) const
		{
			FOR_EACH(const DestPairType pair IN base::GetEnumerator())
				slot(CollectionOp::Added, pair.Key, pair.Value);
		}

		void ChangedHandler(CollectionOp op, const KeyType& key, const SrcType& value)
		{ _onChanged(op, key, base::_caster(value)); }
	};


	namespace Detail
	{

		template < typename SrcDictionaryType >
		class DictionaryCasterProxy
		{
			typedef typename SrcDictionaryType::KeyType KeyType;
			typedef typename SrcDictionaryType::ValueType SrcType;

		private:
			shared_ptr<SrcDictionaryType>	_srcDictionary;

		public:
			DictionaryCasterProxy(const shared_ptr<SrcDictionaryType>& srcDictionary) : _srcDictionary(srcDictionary)
			{ }

			template < typename DestType >
			operator shared_ptr<IReadonlyDictionary<KeyType, DestType> > () const
			{ return make_shared_ptr<DictionaryCaster<KeyType, SrcType, DestType> >(_srcDictionary); }

			template < typename DestType >
			operator shared_ptr<IReadonlyObservableDictionary<KeyType, DestType> > () const
			{ return make_shared_ptr<ObservableDictionaryCaster<KeyType, SrcType, DestType> >(_srcDictionary); }
		};

	}


	template < typename T >
	typename Detail::DictionaryCasterProxy<T> GetDictionaryCaster(const shared_ptr<T>& Dictionary)
	{ return Detail::DictionaryCasterProxy<T>(Dictionary); }

	/** @} */

}

#endif
