#ifndef STINGRAYKIT_COLLECTION_DICTIONARYCASTER_H
#define STINGRAYKIT_COLLECTION_DICTIONARYCASTER_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/ForEach.h>
#include <stingraykit/collection/IObservableDictionary.h>
#include <stingraykit/collection/ITransactionalDictionary.h>
#include <stingraykit/signal/signals.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename KeyType, typename SrcType, typename DestType, typename SrcDictionaryType = IReadonlyDictionary<KeyType, SrcType>>
	class DictionaryCaster : public virtual IReadonlyDictionary<KeyType, DestType>
	{
		using base = IReadonlyDictionary<KeyType, DestType>;
		using Self = DictionaryCaster<KeyType, SrcType, DestType, SrcDictionaryType>;

		STINGRAYKIT_DECLARE_PTR(SrcDictionaryType);

	public:
		using Caster = function<DestType (const SrcType&)>;
		using BackCaster = function<bool (const DestType&, SrcType&)>;

	protected:
		using SrcPairType = typename SrcDictionaryType::PairType;
		using DestPairType = typename base::PairType;

	protected:
		const SrcDictionaryTypePtr							_wrapped;
		const Caster										_caster;
		const BackCaster									_backCaster;

	public:
		DictionaryCaster(const SrcDictionaryTypePtr& wrapped, const Caster& caster, const BackCaster& backCaster)
			: _wrapped(STINGRAYKIT_REQUIRE_NOT_NULL(wrapped)), _caster(caster), _backCaster(backCaster)
		{ }

		shared_ptr<IEnumerator<DestPairType>> GetEnumerator() const override
		{ return WrapEnumerator(_wrapped->GetEnumerator(), Bind(&Self::PairCast, _caster, _1)); }

		shared_ptr<IEnumerable<DestPairType>> Reverse() const override
		{ return WrapEnumerable(_wrapped->Reverse(), Bind(&Self::PairCast, _caster, _1)); }

		size_t GetCount() const override
		{ return _wrapped->GetCount(); }

		bool IsEmpty() const override
		{ return _wrapped->IsEmpty(); }

		bool ContainsKey(const KeyType& key) const override
		{ return _wrapped->ContainsKey(key); }

		shared_ptr<IEnumerator<DestPairType>> Find(const KeyType& key) const override
		{ return WrapEnumerator(_wrapped->Find(key), Bind(&Self::PairCast, _caster, _1)); }

		shared_ptr<IEnumerator<DestPairType>> ReverseFind(const KeyType& key) const override
		{ return WrapEnumerator(_wrapped->ReverseFind(key), Bind(&Self::PairCast, _caster, _1)); }

		DestType Get(const KeyType& key) const override
		{ return _caster(_wrapped->Get(key)); }

		bool TryGet(const KeyType& key, DestType& value) const override
		{
			SrcType value_;
			if (_wrapped->TryGet(key, value_))
			{
				value = _caster(value_);
				return true;
			}
			return false;
		}

	private:
		static DestPairType PairCast(const Caster& caster, const SrcPairType& pair)
		{ return DestPairType(pair.Key, caster(pair.Value)); }
	};


	template < typename KeyType, typename SrcType, typename DestType >
	class ObservableDictionaryCaster : public DictionaryCaster<KeyType, SrcType, DestType, IReadonlyObservableDictionary<KeyType, SrcType>>, public virtual IReadonlyObservableDictionary<KeyType, DestType>
	{
		using base = DictionaryCaster<KeyType, SrcType, DestType, IReadonlyObservableDictionary<KeyType, SrcType>>;
		using Self = ObservableDictionaryCaster<KeyType, SrcType, DestType>;

		using SrcDictionaryType = IReadonlyObservableDictionary<KeyType, SrcType>;
		STINGRAYKIT_DECLARE_PTR(SrcDictionaryType);

		using DestDictionaryType = IReadonlyObservableDictionary<KeyType, DestType>;

		using ExternalMutexPointer = signal_policies::threading::ExternalMutexPointer;

	public:
		using OnChangedSignature = typename DestDictionaryType::OnChangedSignature;

		using DestPairType = typename base::DestPairType;

		using Caster = typename base::Caster;
		using BackCaster = typename base::BackCaster;

	private:
		signal<OnChangedSignature, ExternalMutexPointer>	_onChanged;
		const Token											_connection;

	public:
		ObservableDictionaryCaster(const SrcDictionaryTypePtr& wrapped, const Caster& caster, const BackCaster& backCaster)
			:	base(wrapped, caster, backCaster),
				_onChanged(ExternalMutexPointer(shared_ptr<const Mutex>(base::_wrapped, &base::_wrapped->GetSyncRoot())), Bind(&Self::OnChangedPopulator, this, _1)),
				_connection(base::_wrapped->OnChanged().connect(Bind(&Self::ChangedHandler, this, _1, _2, _3), false))
		{ }

		signal_connector<OnChangedSignature> OnChanged() const override
		{ return _onChanged.connector(); }

		const Mutex& GetSyncRoot() const override
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


	template < typename KeyType, typename SrcType, typename DestType >
	class TransactionalDictionaryCaster : public DictionaryCaster<KeyType, SrcType, DestType, IReadonlyTransactionalDictionary<KeyType, SrcType>>, public virtual IReadonlyTransactionalDictionary<KeyType, DestType>
	{
		using base = DictionaryCaster<KeyType, SrcType, DestType, IReadonlyTransactionalDictionary<KeyType, SrcType>>;
		using Self = TransactionalDictionaryCaster<KeyType, SrcType, DestType>;

		using SrcDictionaryType = IReadonlyTransactionalDictionary<KeyType, SrcType>;
		STINGRAYKIT_DECLARE_PTR(SrcDictionaryType);

		using DestDictionaryType = IReadonlyTransactionalDictionary<KeyType, DestType>;

		using ExternalMutexPointer = signal_policies::threading::ExternalMutexPointer;

	public:
		using OnChangedSignature = typename DestDictionaryType::OnChangedSignature;

		using DestPairType = typename base::DestPairType;

		using Caster = typename base::Caster;
		using BackCaster = typename base::BackCaster;

	private:
		signal<OnChangedSignature, ExternalMutexPointer>	_onChanged;
		const Token											_connection;

	public:
		TransactionalDictionaryCaster(const SrcDictionaryTypePtr& wrapped, const Caster& caster, const BackCaster& backCaster)
			:	base(wrapped, caster, backCaster),
				_onChanged(ExternalMutexPointer(shared_ptr<const Mutex>(base::_wrapped, &base::_wrapped->GetSyncRoot())), Bind(&Self::OnChangedPopulator, this, _1)),
				_connection(base::_wrapped->OnChanged().connect(Bind(&Self::ConvertDiffAndInvoke, this, _onChanged.invoker(), _1), false))
		{ }

		signal_connector<OnChangedSignature> OnChanged() const override
		{ return _onChanged.connector(); }

		const Mutex& GetSyncRoot() const override
		{ return base::_wrapped->GetSyncRoot(); }

	private:
		void OnChangedPopulator(const function<OnChangedSignature>& slot) const
		{ base::_wrapped->OnChanged().SendCurrentState(Bind(&Self::ConvertDiffAndInvoke, this, slot, _1)); }

		void ConvertDiffAndInvoke(const function<OnChangedSignature>& slot, const typename SrcDictionaryType::DiffTypePtr& diff) const
		{ slot(Enumerable::Transform(diff, Bind(&Self::ConvertDiffEntry, base::_caster, _1))); }

		static typename DestDictionaryType::DiffEntryType ConvertDiffEntry(const Caster& caster, const typename SrcDictionaryType::DiffEntryType& entry)
		{ return MakeDiffEntry(entry.Op, typename DestDictionaryType::PairType(entry.Item.Key, caster(entry.Item.Value))); }
	};


	namespace Detail
	{

		template < typename SrcDictionaryType >
		class DictionaryCasterProxy
		{
			using KeyType = typename SrcDictionaryType::KeyType;
			using SrcType = typename SrcDictionaryType::ValueType;

		private:
			shared_ptr<SrcDictionaryType>	_srcDictionary;

		public:
			DictionaryCasterProxy(const shared_ptr<SrcDictionaryType>& srcDictionary) : _srcDictionary(srcDictionary)
			{ }

			template < typename DestType >
			operator shared_ptr<IReadonlyDictionary<KeyType, DestType>> () const
			{ return make_shared_ptr<DictionaryCaster<KeyType, SrcType, DestType>>(_srcDictionary, &DefaultCast<DestType>, &DefaultBackCast<DestType>); }

			template < typename DestType >
			operator shared_ptr<IReadonlyObservableDictionary<KeyType, DestType>> () const
			{ return make_shared_ptr<ObservableDictionaryCaster<KeyType, SrcType, DestType>>(_srcDictionary, &DefaultCast<DestType>, &DefaultBackCast<DestType>); }

			template < typename DestType >
			operator shared_ptr<IReadonlyTransactionalDictionary<KeyType, DestType>> () const
			{ return make_shared_ptr<TransactionalDictionaryCaster<KeyType, SrcType, DestType>>(_srcDictionary, &DefaultCast<DestType>, &DefaultBackCast<DestType>); }

		private:
			template < typename DestType >
			static typename EnableIf<IsConvertible<SrcType, DestType>::Value, DestType>::ValueT DefaultCast(const SrcType& value)
			{ return DestType(value); }

			template < typename DestType >
			static typename EnableIf<IsSharedPtr<SrcType>::Value && IsSharedPtr<DestType>::Value, bool>::ValueT DefaultBackCast(const DestType& value, SrcType& result)
			{
				result = dynamic_caster(value);
				return result.is_initialized();
			}
		};

	}


	template < typename T >
	typename Detail::DictionaryCasterProxy<T> GetDictionaryCaster(const shared_ptr<T>& Dictionary)
	{ return Detail::DictionaryCasterProxy<T>(Dictionary); }

	/** @} */

}

#endif
