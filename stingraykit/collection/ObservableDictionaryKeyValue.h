#ifndef STINGRAYKIT_COLLECTION_OBSERVABLEDICTIONARYKEYVALUE_H
#define STINGRAYKIT_COLLECTION_OBSERVABLEDICTIONARYKEYVALUE_H

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
			ValueType value;
			_dict->TryGet(_key, value);
			return value;
		}

		virtual signal_connector<OnChangedSignature> OnChanged() const
		{ return _onChanged.connector(); }

		virtual const Mutex& GetSyncRoot() const
		{ return _dict->GetSyncRoot(); }

	private:
		void InvokeOnChanged(CollectionOp op, const KeyType_& key, const ValueType_& value)
		{
			if (EqualsCmp_()(_key, key))
				_onChanged(op == CollectionOp::Removed ? null : value);
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
				_dict->Set(_key, value);
			else
				_dict->TryRemove(_key);
		}
	};


	template < typename KeyType, typename ValueType, typename EqualsCmp >
	shared_ptr<IReadonlyObservableValue<ValueType> > GetDictionaryKeyValue(const shared_ptr<IReadonlyObservableDictionary<KeyType, ValueType> >& dict, const KeyType& key, const EqualsCmp& equalsCmp)
	{ return make_shared<ReadonlyObservableDictionaryKeyValue<KeyType, ValueType, EqualsCmp> >(dict, key); }


	template < typename KeyType, typename ValueType, typename EqualsCmp >
	shared_ptr<IObservableValue<ValueType> > GetDictionaryKeyValue(const shared_ptr<IObservableDictionary<KeyType, ValueType> >& dict, const KeyType& key, const EqualsCmp& equalsCmp)
	{ return make_shared<ObservableDictionaryKeyValue<KeyType, ValueType, EqualsCmp> >(dict, key); }

}

#endif
