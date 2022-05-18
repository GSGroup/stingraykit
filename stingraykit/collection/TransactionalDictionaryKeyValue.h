#ifndef STINGRAYKIT_COLLECTION_TRANSACTIONALDICTIONARYKEYVALUE_H
#define STINGRAYKIT_COLLECTION_TRANSACTIONALDICTIONARYKEYVALUE_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/ForEach.h>
#include <stingraykit/collection/ITransactionalDictionary.h>
#include <stingraykit/signal/signals.h>
#include <stingraykit/IObservableValue.h>

namespace stingray
{

	template < typename KeyType_, typename ValueType_, typename EqualsCmp_ >
	class ReadonlyTransactionalDictionaryKeyValue
		:	public virtual IReadonlyObservableValue<typename If<IsNullable<ValueType_>::Value, ValueType_, optional<ValueType_>>::ValueT>
	{
		using Self = ReadonlyTransactionalDictionaryKeyValue;

		using ValueType = typename If<IsNullable<ValueType_>::Value, ValueType_, optional<ValueType_>>::ValueT;

		using Base = IReadonlyObservableValue<ValueType>;

		using ParamPassingType = typename Base::ParamPassingType;
		using OnChangedSignature = typename Base::OnChangedSignature;

		using DictionaryType = IReadonlyTransactionalDictionary<KeyType_, ValueType_>;
		STINGRAYKIT_DECLARE_PTR(DictionaryType);

		using ExternalMutexPointer = signal_policies::threading::ExternalMutexPointer;

	private:
		DictionaryTypePtr									_dict;
		KeyType_											_key;
		EqualsCmp_											_comparer;

		signal<OnChangedSignature, ExternalMutexPointer>	_onChanged;
		Token												_connection;

	public:
		ReadonlyTransactionalDictionaryKeyValue(const DictionaryTypePtr& dict, const KeyType_& key, const EqualsCmp_& comparer)
			:	_dict(STINGRAYKIT_REQUIRE_NOT_NULL(dict)),
				_key(key),
				_comparer(comparer),
				_onChanged(ExternalMutexPointer(shared_ptr<const Mutex>(_dict, &_dict->GetSyncRoot())), Bind(&Self::OnChangedPopulator, this, _1)),
				_connection(_dict->OnChanged().connect(Bind(&Self::InvokeOnChanged, this, _1), false))
		{ }

		ValueType Get() const override
		{
			ValueType_ value;
			return _dict->TryGet(_key, value) ? ValueType(value) : null;
		}

		signal_connector<OnChangedSignature> OnChanged() const override
		{ return _onChanged.connector(); }

		const Mutex& GetSyncRoot() const override
		{ return _dict->GetSyncRoot(); }

	private:
		void InvokeOnChanged(const typename DictionaryType::DiffTypePtr& diff)
		{
			bool wasChanged = false;
			ValueType value;

			FOR_EACH(const typename DictionaryType::DiffEntryType entry IN diff WHERE _comparer(_key, entry.Item.Key))
			{
				wasChanged = true;
				value = entry.Op == CollectionOp::Removed ? null : ValueType(entry.Item.Value);
			}

			if (wasChanged)
				_onChanged(value);
		}

		void OnChangedPopulator(const function<OnChangedSignature>& slot) const
		{ slot(Get()); }
	};


	template < typename DictionaryType, typename EqualsCmp, typename EnableIf<IsInheritedIReadonlyTransactionalDictionary<DictionaryType>::Value, int>::ValueT = 0 >
	auto GetDictionaryKeyValue(const shared_ptr<DictionaryType>& dict, const typename DictionaryType::KeyType& key, const EqualsCmp& equalsCmp)
	{ return make_shared_ptr<ReadonlyTransactionalDictionaryKeyValue<typename DictionaryType::KeyType, typename DictionaryType::ValueType, EqualsCmp>>(dict, key, equalsCmp); }


	template < typename DictionaryType, typename EnableIf<IsInheritedIReadonlyTransactionalDictionary<DictionaryType>::Value, int>::ValueT = 0 >
	auto GetDictionaryKeyValue(const shared_ptr<DictionaryType>& dict, const typename DictionaryType::KeyType& key)
	{ return GetDictionaryKeyValue(dict, key, comparers::Equals()); }

}

#endif
