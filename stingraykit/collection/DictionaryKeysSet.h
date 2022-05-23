#ifndef STINGRAYKIT_COLLECTION_DICTIONARYKEYSSET_H
#define STINGRAYKIT_COLLECTION_DICTIONARYKEYSSET_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/IObservableDictionary.h>
#include <stingraykit/collection/IObservableSet.h>
#include <stingraykit/collection/KeyValueEnumerableHelpers.h>
#include <stingraykit/signal/signals.h>

namespace stingray
{

	template < typename KeyType_, typename ValueType_ >
	class DictionaryKeysSet : public virtual IReadonlySet<KeyType_>
	{
		using DictionaryType = IReadonlyDictionary<KeyType_, ValueType_>;
		STINGRAYKIT_DECLARE_PTR(DictionaryType);

		using ValueType = KeyType_;

	private:
		const DictionaryTypePtr		_dict;

	public:
		explicit DictionaryKeysSet(const DictionaryTypePtr& dict)
			:	_dict(STINGRAYKIT_REQUIRE_NOT_NULL(dict))
		{ }

		shared_ptr<IEnumerator<ValueType>> GetEnumerator() const override
		{ return KeysEnumerator(_dict->GetEnumerator()); }

		shared_ptr<IEnumerable<ValueType>> Reverse() const override
		{ return KeysEnumerable(_dict->Reverse()); }

		size_t GetCount() const override
		{ return _dict->GetCount(); }

		bool IsEmpty() const override
		{ return _dict->IsEmpty(); }

		bool Contains(const ValueType& value) const override
		{ return _dict->ContainsKey(value); }

		shared_ptr<IEnumerator<ValueType>> Find(const ValueType& value) const override
		{ return KeysEnumerator(_dict->Find(value)); }

		shared_ptr<IEnumerator<ValueType>> ReverseFind(const ValueType& value) const override
		{ return KeysEnumerator(_dict->ReverseFind(value)); }
	};


	template < typename KeyType_, typename ValueType_ >
	class ObservableDictionaryKeysSet : public virtual IReadonlyObservableSet<KeyType_>
	{
		using SetType = IReadonlyObservableSet<KeyType_>;
		using OnChangedSignature = typename SetType::OnChangedSignature;

		using ExternalMutexPointer = signal_policies::threading::ExternalMutexPointer;

		using DictionaryType = IReadonlyObservableDictionary<KeyType_, ValueType_>;
		STINGRAYKIT_DECLARE_PTR(DictionaryType);

		using ValueType = KeyType_;

	private:
		const DictionaryTypePtr								_dict;
		signal<OnChangedSignature, ExternalMutexPointer>	_onChanged;
		const Token											_connection;

	public:
		explicit ObservableDictionaryKeysSet(const DictionaryTypePtr& dict)
			:	_dict(STINGRAYKIT_REQUIRE_NOT_NULL(dict)),
				_onChanged(ExternalMutexPointer(shared_ptr<const Mutex>(_dict, &_dict->GetSyncRoot())), Bind(&ObservableDictionaryKeysSet::OnChangedPopulator, this, _1)),
				_connection(_dict->OnChanged().connect(Bind(&ObservableDictionaryKeysSet::InvokeOnChanged, this, _1, _2)))
		{ }

		shared_ptr<IEnumerator<ValueType>> GetEnumerator() const override
		{ return KeysEnumerator(_dict->GetEnumerator()); }

		shared_ptr<IEnumerable<ValueType>> Reverse() const override
		{ return KeysEnumerable(_dict->Reverse()); }

		size_t GetCount() const override
		{ return _dict->GetCount(); }

		bool IsEmpty() const override
		{ return _dict->IsEmpty(); }

		bool Contains(const ValueType& value) const override
		{ return _dict->ContainsKey(value); }

		shared_ptr<IEnumerator<ValueType>> Find(const ValueType& value) const override
		{ return KeysEnumerator(_dict->Find(value)); }

		shared_ptr<IEnumerator<ValueType>> ReverseFind(const ValueType& value) const override
		{ return KeysEnumerator(_dict->ReverseFind(value)); }

		signal_connector<OnChangedSignature> OnChanged() const override
		{ return _onChanged.connector(); }

		const Mutex& GetSyncRoot() const override
		{ return _dict->GetSyncRoot(); }

	private:
		void InvokeOnChanged(CollectionOp op, const ValueType& val)
		{
			if (op != CollectionOp::Updated)
				_onChanged(op, val);
		}

		void OnChangedPopulator(const function<OnChangedSignature> slot) const
		{ _dict->OnChanged().SendCurrentState(Bind(slot, _1, _2)); }
	};


	template < typename DictionaryType >
	shared_ptr<IReadonlySet<typename DictionaryType::KeyType>> GetDictionaryKeys(const shared_ptr<DictionaryType>& dict, typename EnableIf<!IsInherited2ParamTemplate<DictionaryType, IObservableDictionary>::Value, int>::ValueT dummy = 0)
	{ return make_shared_ptr<DictionaryKeysSet<typename DictionaryType::KeyType, typename DictionaryType::ValueType>>(dict); }

	template < typename DictionaryType >
	shared_ptr<IReadonlyObservableSet<typename DictionaryType::KeyType>> GetDictionaryKeys(const shared_ptr<DictionaryType>& dict, typename EnableIf<IsInherited2ParamTemplate<DictionaryType, IObservableDictionary>::Value, int>::ValueT dummy = 0)
	{ return make_shared_ptr<ObservableDictionaryKeysSet<typename DictionaryType::KeyType, typename DictionaryType::ValueType>>(dict); }

}

#endif
