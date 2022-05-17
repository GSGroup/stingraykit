#ifndef STINGRAYKIT_COLLECTION_OBSERVABLEDICTIONARYKEYVALUE_H
#define STINGRAYKIT_COLLECTION_OBSERVABLEDICTIONARYKEYVALUE_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/IObservableDictionary.h>
#include <stingraykit/signal/signals.h>
#include <stingraykit/IObservableValue.h>

namespace stingray
{

	template < typename KeyType_, typename ValueType_, typename EqualsCmp_ >
	class ReadonlyObservableDictionaryKeyValue
		:	public virtual IReadonlyObservableValue<typename If<IsNullable<ValueType_>::Value, ValueType_, optional<ValueType_>>::ValueT>
	{
		using Self = ReadonlyObservableDictionaryKeyValue;

		using ValueType = typename If<IsNullable<ValueType_>::Value, ValueType_, optional<ValueType_>>::ValueT;

		using Base = IReadonlyObservableValue<ValueType>;

		using ParamPassingType = typename Base::ParamPassingType;
		using OnChangedSignature = typename Base::OnChangedSignature;

		using DictionaryType = IReadonlyObservableDictionary<KeyType_, ValueType_>;
		STINGRAYKIT_DECLARE_PTR(DictionaryType);

		using ExternalMutexPointer = signal_policies::threading::ExternalMutexPointer;

	private:
		DictionaryTypePtr									_dict;
		KeyType_											_key;
		EqualsCmp_											_comparer;

		signal<OnChangedSignature, ExternalMutexPointer>	_onChanged;
		Token												_connection;

	public:
		ReadonlyObservableDictionaryKeyValue(const DictionaryTypePtr& dict, const KeyType_& key, const EqualsCmp_& comparer)
			:	_dict(STINGRAYKIT_REQUIRE_NOT_NULL(dict)),
				_key(key),
				_comparer(comparer),
				_onChanged(ExternalMutexPointer(shared_ptr<const Mutex>(_dict, &_dict->GetSyncRoot())), Bind(&Self::OnChangedPopulator, this, _1)),
				_connection(_dict->OnChanged().connect(Bind(&Self::InvokeOnChanged, this, _1, _2, _3), false))
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
		void InvokeOnChanged(CollectionOp op, const KeyType_& key, const ValueType_& value)
		{
			if (_comparer(_key, key))
				_onChanged(op == CollectionOp::Removed ? null : ValueType(value));
		}

		void OnChangedPopulator(const function<OnChangedSignature>& slot) const
		{ slot(Get()); }
	};


	template < typename KeyType_, typename ValueType_, typename EqualsCmp_ >
	class ObservableDictionaryKeyValue
		:	public virtual IObservableValue<typename If<IsNullable<ValueType_>::Value, ValueType_, optional<ValueType_>>::ValueT>,
			public ReadonlyObservableDictionaryKeyValue<KeyType_, ValueType_, EqualsCmp_>
	{
		using ValueType = typename If<IsNullable<ValueType_>::Value, ValueType_, optional<ValueType_>>::ValueT;

		using DictionaryType = IObservableDictionary<KeyType_, ValueType_>;
		STINGRAYKIT_DECLARE_PTR(DictionaryType);

		using Base = IObservableValue<ValueType>;

		using ParamPassingType = typename Base::ParamPassingType;

	private:
		DictionaryTypePtr		_dict;
		KeyType_				_key;

	public:
		ObservableDictionaryKeyValue(const DictionaryTypePtr& dict, const KeyType_& key, const EqualsCmp_& comparer)
			:	ReadonlyObservableDictionaryKeyValue<KeyType_, ValueType_, EqualsCmp_>(dict, key, comparer),
				_dict(dict),
				_key(key)
		{ }

		void Set(ParamPassingType value) override
		{
			if (value)
				DoSet<ValueType_>(value);
			else
				_dict->TryRemove(_key);
		}

	private:
		template < typename ValueType >
		void DoSet(const typename EnableIf<IsNullable<ValueType>::Value, ParamPassingType>::ValueT& value)
		{ _dict->Set(_key, value); }

		template < typename ValueType >
		void DoSet(const typename EnableIf<!IsNullable<ValueType>::Value, ParamPassingType>::ValueT& value)
		{ _dict->Set(_key, *value); }
	};


	template < typename KeyType, typename ValueType, typename EqualsCmp >
	shared_ptr<ReadonlyObservableDictionaryKeyValue<KeyType, ValueType, EqualsCmp>> GetDictionaryKeyValue(const shared_ptr<IReadonlyObservableDictionary<KeyType, ValueType>>& dict, const KeyType& key, const EqualsCmp& equalsCmp)
	{ return make_shared_ptr<ReadonlyObservableDictionaryKeyValue<KeyType, ValueType, EqualsCmp>>(dict, key, equalsCmp); }


	template < typename KeyType, typename ValueType >
	shared_ptr<ReadonlyObservableDictionaryKeyValue<KeyType, ValueType, comparers::Equals>> GetDictionaryKeyValue(const shared_ptr<IReadonlyObservableDictionary<KeyType, ValueType>>& dict, const KeyType& key)
	{ return GetDictionaryKeyValue(dict, key, comparers::Equals()); }


	template < typename KeyType, typename ValueType, typename EqualsCmp >
	shared_ptr<ObservableDictionaryKeyValue<KeyType, ValueType, EqualsCmp>> GetDictionaryKeyValue(const shared_ptr<IObservableDictionary<KeyType, ValueType>>& dict, const KeyType& key, const EqualsCmp& equalsCmp)
	{ return make_shared_ptr<ObservableDictionaryKeyValue<KeyType, ValueType, EqualsCmp>>(dict, key, equalsCmp); }


	template < typename KeyType, typename ValueType >
	shared_ptr<ObservableDictionaryKeyValue<KeyType, ValueType, comparers::Equals>> GetDictionaryKeyValue(const shared_ptr<IObservableDictionary<KeyType, ValueType>>& dict, const KeyType& key)
	{ return GetDictionaryKeyValue(dict, key, comparers::Equals()); }

}

#endif
