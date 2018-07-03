#ifndef STINGRAYKIT_COLLECTION_OBSERVABLEDICTIONARYKEYVALUE_H
#define STINGRAYKIT_COLLECTION_OBSERVABLEDICTIONARYKEYVALUE_H

// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
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
		:	public virtual IReadonlyObservableValue<typename If<IsNullable<ValueType_>::Value, ValueType_, optional<ValueType_> >::ValueT>
	{
		typedef ReadonlyObservableDictionaryKeyValue Self;

		typedef typename If<IsNullable<ValueType_>::Value, ValueType_, optional<ValueType_> >::ValueT ValueType;

		typedef IReadonlyObservableValue<ValueType> Base;

		typedef typename Base::ParamPassingType ParamPassingType;
		typedef typename Base::OnChangedSignature OnChangedSignature;

		typedef IReadonlyObservableDictionary<KeyType_, ValueType_> DictionaryType;
		STINGRAYKIT_DECLARE_PTR(DictionaryType);

		typedef signal_policies::threading::ExternalMutexPointer ExternalMutexPointer;

	private:
		DictionaryTypePtr									_dict;
		KeyType_											_key;

		signal<OnChangedSignature, ExternalMutexPointer>	_onChanged;
		Token												_connection;

	public:
		ReadonlyObservableDictionaryKeyValue(const DictionaryTypePtr& dict, const KeyType_& key)
			:	_dict(STINGRAYKIT_REQUIRE_NOT_NULL(dict)),
				_key(key),
				_onChanged(ExternalMutexPointer(shared_ptr<const Mutex>(_dict, &_dict->GetSyncRoot())), bind(&Self::OnChangedPopulator, this, _1)),
				_connection(_dict->OnChanged().connect(bind(&Self::InvokeOnChanged, this, _1, _2, _3), false))
		{ }

		virtual ValueType Get() const
		{
			ValueType_ value;
			return _dict->TryGet(_key, value) ? ValueType(value) : null;
		}

		virtual signal_connector<OnChangedSignature> OnChanged() const
		{ return _onChanged.connector(); }

		virtual const Mutex& GetSyncRoot() const
		{ return _dict->GetSyncRoot(); }

	private:
		void InvokeOnChanged(CollectionOp op, const KeyType_& key, const ValueType_& value)
		{
			if (EqualsCmp_()(_key, key))
				_onChanged(op == CollectionOp::Removed ? null : ValueType(value));
		}

		void OnChangedPopulator(const function<OnChangedSignature>& slot) const
		{ slot(Get()); }
	};


	template < typename KeyType_, typename ValueType_, typename EqualsCmp_ >
	class ObservableDictionaryKeyValue
		:	public virtual IObservableValue<typename If<IsNullable<ValueType_>::Value, ValueType_, optional<ValueType_> >::ValueT>,
			public ReadonlyObservableDictionaryKeyValue<KeyType_, ValueType_, EqualsCmp_>
	{
		typedef typename If<IsNullable<ValueType_>::Value, ValueType_, optional<ValueType_> >::ValueT ValueType;

		typedef IObservableDictionary<KeyType_, ValueType_> DictionaryType;
		STINGRAYKIT_DECLARE_PTR(DictionaryType);

		typedef IObservableValue<ValueType> Base;

		typedef typename Base::ParamPassingType ParamPassingType;

	private:
		DictionaryTypePtr		_dict;
		KeyType_				_key;

	public:
		ObservableDictionaryKeyValue(const DictionaryTypePtr& dict, const KeyType_& key)
			:	ReadonlyObservableDictionaryKeyValue<KeyType_, ValueType_, EqualsCmp_>(dict, key),
				_dict(dict),
				_key(key)
		{ }

		virtual void Set(const ParamPassingType& value)
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
	shared_ptr<ReadonlyObservableDictionaryKeyValue<KeyType, ValueType, EqualsCmp> > GetDictionaryKeyValue(const shared_ptr<IReadonlyObservableDictionary<KeyType, ValueType> >& dict, const KeyType& key, const EqualsCmp& equalsCmp)
	{ return make_shared<ReadonlyObservableDictionaryKeyValue<KeyType, ValueType, EqualsCmp> >(dict, key); }


	template < typename KeyType, typename ValueType >
	shared_ptr<ReadonlyObservableDictionaryKeyValue<KeyType, ValueType, comparers::Equals> > GetDictionaryKeyValue(const shared_ptr<IReadonlyObservableDictionary<KeyType, ValueType> >& dict, const KeyType& key)
	{ return GetDictionaryKeyValue(dict, key, comparers::Equals()); }


	template < typename KeyType, typename ValueType, typename EqualsCmp >
	shared_ptr<ObservableDictionaryKeyValue<KeyType, ValueType, EqualsCmp> > GetDictionaryKeyValue(const shared_ptr<IObservableDictionary<KeyType, ValueType> >& dict, const KeyType& key, const EqualsCmp& equalsCmp)
	{ return make_shared<ObservableDictionaryKeyValue<KeyType, ValueType, EqualsCmp> >(dict, key); }


	template < typename KeyType, typename ValueType >
	shared_ptr<ObservableDictionaryKeyValue<KeyType, ValueType, comparers::Equals> > GetDictionaryKeyValue(const shared_ptr<IObservableDictionary<KeyType, ValueType> >& dict, const KeyType& key)
	{ return GetDictionaryKeyValue(dict, key, comparers::Equals()); }

}

#endif
