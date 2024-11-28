#ifndef STINGRAYKIT_COLLECTION_DICTIONARYKEYSSET_H
#define STINGRAYKIT_COLLECTION_DICTIONARYKEYSSET_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/IObservableDictionary.h>
#include <stingraykit/collection/IObservableSet.h>
#include <stingraykit/collection/ITransactionalDictionary.h>
#include <stingraykit/collection/ITransactionalSet.h>
#include <stingraykit/collection/KeyValueEnumerableHelpers.h>
#include <stingraykit/signal/signals.h>

namespace stingray
{

	template < typename KeyType_, typename ValueType_, typename DictionaryType_ = IReadonlyDictionary<KeyType_, ValueType_> >
	class DictionaryKeysSet : public virtual IReadonlySet<KeyType_>
	{
		using DictionaryType = DictionaryType_;
		STINGRAYKIT_DECLARE_PTR(DictionaryType);

		using ValueType = KeyType_;

	protected:
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
	class ObservableDictionaryKeysSet : public DictionaryKeysSet<KeyType_, ValueType_, IReadonlyObservableDictionary<KeyType_, ValueType_>>, public virtual IReadonlyObservableSet<KeyType_>
	{
		using base = DictionaryKeysSet<KeyType_, ValueType_, IReadonlyObservableDictionary<KeyType_, ValueType_>>;

		using SetType = IReadonlyObservableSet<KeyType_>;
		using OnChangedSignature = typename SetType::OnChangedSignature;

		using ExternalMutexPointer = signal_policies::threading::ExternalMutexPointer;

		using DictionaryType = IReadonlyObservableDictionary<KeyType_, ValueType_>;
		STINGRAYKIT_DECLARE_PTR(DictionaryType);

	private:
		signal<OnChangedSignature, ExternalMutexPointer>	_onChanged;
		const Token											_connection;

	public:
		explicit ObservableDictionaryKeysSet(const DictionaryTypePtr& dict)
			:	base(dict),
				_onChanged(ExternalMutexPointer(shared_ptr<const Mutex>(base::_dict, &base::_dict->GetSyncRoot())), Bind(&ObservableDictionaryKeysSet::OnChangedPopulator, this, _1)),
				_connection(base::_dict->OnChanged().connect(Bind(&ObservableDictionaryKeysSet::InvokeOnChanged, this, _1, _2), false))
		{ }

		signal_connector<OnChangedSignature> OnChanged() const override
		{ return _onChanged.connector(); }

		const Mutex& GetSyncRoot() const override
		{ return base::_dict->GetSyncRoot(); }

	private:
		void InvokeOnChanged(CollectionOp op, const KeyType_& val)
		{
			if (op != CollectionOp::Updated)
				_onChanged(op, val);
		}

		void OnChangedPopulator(const function<OnChangedSignature> slot) const
		{ base::_dict->OnChanged().SendCurrentState(Bind(slot, _1, _2)); }
	};


	template < typename KeyType_, typename ValueType_, typename EqualsCmp_ >
	class TransactionalDictionaryKeysSet : public DictionaryKeysSet<KeyType_, ValueType_, IReadonlyTransactionalDictionary<KeyType_, ValueType_>>, public virtual IReadonlyTransactionalSet<KeyType_>
	{
		using base = DictionaryKeysSet<KeyType_, ValueType_, IReadonlyTransactionalDictionary<KeyType_, ValueType_>>;

		using SetType = IReadonlyTransactionalSet<KeyType_>;

		using DiffEntryType = typename SetType::DiffEntryType;
		using OnChangedSignature = typename SetType::OnChangedSignature;

		using ExternalMutexPointer = signal_policies::threading::ExternalMutexPointer;

		using DictionaryType = IReadonlyTransactionalDictionary<KeyType_, ValueType_>;
		STINGRAYKIT_DECLARE_PTR(DictionaryType);

		class DiffEnumerator : public virtual IEnumerator<DiffEntryType>
		{
			using SrcDiffEntryType = typename DictionaryType::DiffEntryType;
			using SrcDiffEnumerator = IEnumerator<SrcDiffEntryType>;
			STINGRAYKIT_DECLARE_PTR(SrcDiffEnumerator);

		private:
			SrcDiffEnumeratorPtr			_src;
			EqualsCmp_						_comparer;

			optional<SrcDiffEntryType>		_entry;

		public:
			DiffEnumerator(const SrcDiffEnumeratorPtr& src, const EqualsCmp_& comparer)
				:	_src(STINGRAYKIT_REQUIRE_NOT_NULL(src)),
					_comparer(comparer)
			{ FindNext(); }

			bool Valid() const override
			{ return _entry.is_initialized(); }

			DiffEntryType Get() const override
			{
				STINGRAYKIT_CHECK(Valid(), "Enumerator is not valid!");
				return MakeDiffEntry(_entry->Op, _entry->Item.Key);
			}

			void Next() override
			{
				STINGRAYKIT_CHECK(Valid(), "Enumerator is not valid!");
				_entry.reset();
				FindNext();
			}

		private:
			void FindNext()
			{
				while (_src->Valid())
				{
					if (!_entry)
					{
						_entry = CheckDiffEntry(_src->Get());
						_src->Next();
					}
					else
					{
						const SrcDiffEntryType nextEntry = CheckDiffEntry(_src->Get());

						if (!_comparer(_entry->Item.Key, nextEntry.Item.Key))
							break;

						STINGRAYKIT_CHECK(_entry->Op == CollectionOp::Removed && nextEntry.Op == CollectionOp::Added, "Unexpected collection op sequence");

						_entry.reset();
						_src->Next();
					}
				}
			}

			static SrcDiffEntryType CheckDiffEntry(const SrcDiffEntryType& entry)
			{
				STINGRAYKIT_CHECK(entry.Op == CollectionOp::Added || entry.Op == CollectionOp::Removed, "Unexpected collection op");
				return entry;
			}
		};

	private:
		EqualsCmp_											_comparer;
		signal<OnChangedSignature, ExternalMutexPointer>	_onChanged;
		const Token											_connection;

	public:
		TransactionalDictionaryKeysSet(const DictionaryTypePtr& dict, const EqualsCmp_& comparer)
			:	base(dict),
				_comparer(comparer),
				_onChanged(ExternalMutexPointer(shared_ptr<const Mutex>(base::_dict, &base::_dict->GetSyncRoot())), Bind(&TransactionalDictionaryKeysSet::OnChangedPopulator, this, _1)),
				_connection(base::_dict->OnChanged().connect(Bind(&TransactionalDictionaryKeysSet::ConvertDiffAndInvoke, this, _onChanged.invoker(), _1), false))
		{ }

		signal_connector<OnChangedSignature> OnChanged() const override
		{ return _onChanged.connector(); }

		const Mutex& GetSyncRoot() const override
		{ return base::_dict->GetSyncRoot(); }

	private:
		void OnChangedPopulator(const function<OnChangedSignature>& slot) const
		{ base::_dict->OnChanged().SendCurrentState(Bind(&TransactionalDictionaryKeysSet::ConvertDiffAndInvoke, this, slot, _1)); }

		void ConvertDiffAndInvoke(const function<OnChangedSignature>& slot, const typename DictionaryType::DiffTypePtr& srcDiff) const
		{
			const typename SetType::DiffTypePtr diff = MakeSimpleEnumerable(Bind(MakeShared<DiffEnumerator>(), Bind(&DictionaryType::DiffType::GetEnumerator, srcDiff), _comparer));

			if (Enumerable::Any(diff))
				slot(diff);
		}
	};


	template < typename DictionaryType,
			typename EnableIf<IsInherited2ParamTemplate<DictionaryType, IReadonlyDictionary>::Value &&
					!IsInherited2ParamTemplate<DictionaryType, IReadonlyObservableDictionary>::Value &&
					!IsInherited2ParamTemplate<DictionaryType, IReadonlyTransactionalDictionary>::Value, int>::ValueT = 0 >
	shared_ptr<IReadonlySet<typename DictionaryType::KeyType>> GetDictionaryKeys(const shared_ptr<DictionaryType>& dict)
	{ return make_shared_ptr<DictionaryKeysSet<typename DictionaryType::KeyType, typename DictionaryType::ValueType>>(dict); }

	template < typename DictionaryType, typename EnableIf<IsInherited2ParamTemplate<DictionaryType, IReadonlyObservableDictionary>::Value, int>::ValueT = 0 >
	shared_ptr<IReadonlyObservableSet<typename DictionaryType::KeyType>> GetDictionaryKeys(const shared_ptr<DictionaryType>& dict)
	{ return make_shared_ptr<ObservableDictionaryKeysSet<typename DictionaryType::KeyType, typename DictionaryType::ValueType>>(dict); }

	template < typename DictionaryType, typename EqualsCmp, typename EnableIf<IsInherited2ParamTemplate<DictionaryType, IReadonlyTransactionalDictionary>::Value, int>::ValueT = 0 >
	shared_ptr<IReadonlyTransactionalSet<typename DictionaryType::KeyType>> GetDictionaryKeys(const shared_ptr<DictionaryType>& dict, const EqualsCmp& equalsCmp)
	{ return make_shared_ptr<TransactionalDictionaryKeysSet<typename DictionaryType::KeyType, typename DictionaryType::ValueType, EqualsCmp>>(dict, equalsCmp); }

	template < typename DictionaryType, typename EnableIf<IsInherited2ParamTemplate<DictionaryType, IReadonlyTransactionalDictionary>::Value, int>::ValueT = 0 >
	shared_ptr<IReadonlyTransactionalSet<typename DictionaryType::KeyType>> GetDictionaryKeys(const shared_ptr<DictionaryType>& dict)
	{ return GetDictionaryKeys(dict, comparers::Equals()); }

}

#endif
