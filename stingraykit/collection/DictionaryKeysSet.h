#ifndef STINGRAYKIT_COLLECTION_DICTIONARYKEYSSET_H
#define STINGRAYKIT_COLLECTION_DICTIONARYKEYSSET_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumeratorWrapper.h>
#include <stingraykit/collection/IObservableDictionary.h>
#include <stingraykit/collection/IObservableSet.h>
#include <stingraykit/signal/signals.h>

namespace stingray
{

	template < typename KeyType_, typename ValueType_ >
	class DictionaryKeysSet : public virtual IReadonlySet<KeyType_>
	{
		STINGRAYKIT_NONCOPYABLE(DictionaryKeysSet);

		typedef IDictionary<KeyType_, ValueType_> DictionaryType;
		STINGRAYKIT_DECLARE_PTR(DictionaryType);

		typedef KeyType_	ValueType; // Dictionary KeyType is ValueType for set

	private:
		DictionaryTypePtr	_dict;

	public:
		DictionaryKeysSet(const DictionaryTypePtr& dict)
			: _dict(dict)
		{ }

		virtual shared_ptr<IEnumerator<ValueType> > GetEnumerator() const
		{ return make_shared<EnumeratorWrapper<typename DictionaryType::PairType, ValueType> >(_dict->GetEnumerator(), bind(&DictionaryType::PairType::GetKey, _1)); }

		virtual shared_ptr<IEnumerable<ValueType> > Reverse() const
		{ return make_shared<EnumerableWrapper<typename DictionaryType::PairType, ValueType> >(_dict->Reverse(), bind(&DictionaryType::PairType::GetKey, _1)); }

		virtual size_t GetCount() const
		{ return _dict->GetCount(); }

		virtual bool IsEmpty() const
		{ return _dict->IsEmpty(); }

		virtual bool Contains(const ValueType& value) const
		{ return _dict->ContainsKey(value); }
	};


	template < typename KeyType_, typename ValueType_ >
	class ObservableDictionaryKeysSet : public virtual IReadonlyObservableSet<KeyType_>
	{
		STINGRAYKIT_NONCOPYABLE(ObservableDictionaryKeysSet);

		typedef IObservableDictionary<KeyType_, ValueType_> DictionaryType;
		STINGRAYKIT_DECLARE_PTR(DictionaryType);

		typedef KeyType_	ValueType; // Dictionary KeyType is ValueType for set

	private:
		DictionaryTypePtr																		_dict;
		signal<void(CollectionOp, const ValueType&), signal_policies::threading::ExternalMutex>	_onChanged;
		TokenPool																				_connections;

	public:
		ObservableDictionaryKeysSet(const DictionaryTypePtr& dict)
			: _dict(dict), _onChanged(signal_policies::threading::ExternalMutex(_dict->GetSyncRoot()), bind(&ObservableDictionaryKeysSet::OnChangedPopulator, this, _1))
		{ _connections += _dict->OnChanged().connect(bind(&ObservableDictionaryKeysSet::InvokeOnChanged, this, _1, _2, not_using(_3))); }

		~ObservableDictionaryKeysSet()
		{ _connections.Release(); }

		virtual shared_ptr<IEnumerator<ValueType> > GetEnumerator() const
		{ return KeysEnumerator(_dict->GetEnumerator()); }

		virtual shared_ptr<IEnumerable<ValueType> > Reverse() const
		{ return KeysEnumerable(_dict->Reverse()); }

		virtual size_t GetCount() const
		{ return _dict->GetCount(); }

		virtual bool IsEmpty() const
		{ return _dict->IsEmpty(); }

		virtual bool Contains(const ValueType& value) const
		{ return _dict->ContainsKey(value); }

		virtual signal_connector<void(CollectionOp, const ValueType&)> OnChanged() const
		{ return _onChanged.connector(); }

		virtual const Mutex& GetSyncRoot() const
		{ return _dict->GetSyncRoot(); }

	private:
		virtual void InvokeOnChanged(CollectionOp op, const ValueType& val)
		{ if (op != CollectionOp::Updated) _onChanged(op, val); }

		virtual void OnChangedPopulator(const function<void(CollectionOp, const ValueType&)> slot) const
		{ _dict->OnChanged().SendCurrentState(bind(slot, _1, _2, not_using(_3))); }
	};


	template < typename DictionaryType >
	shared_ptr<DictionaryKeysSet<typename DictionaryType::KeyType, typename DictionaryType::ValueType> > GetDictionaryKeys(const shared_ptr<DictionaryType>& dict, typename EnableIf<!Inherits2ParamTemplate<DictionaryType, IObservableDictionary>::Value, int>::ValueT dummy = 0)
	{ return make_shared<DictionaryKeysSet<typename DictionaryType::KeyType, typename DictionaryType::ValueType> >(dict); }

	template < typename DictionaryType >
	shared_ptr<ObservableDictionaryKeysSet<typename DictionaryType::KeyType, typename DictionaryType::ValueType> > GetDictionaryKeys(const shared_ptr<DictionaryType>& dict, typename EnableIf<Inherits2ParamTemplate<DictionaryType, IObservableDictionary>::Value, int>::ValueT dummy = 0)
	{ return make_shared<ObservableDictionaryKeysSet<typename DictionaryType::KeyType, typename DictionaryType::ValueType> >(dict); }

}

#endif
