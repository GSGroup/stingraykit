#ifndef STINGRAYKIT_COLLECTION_TRANSACTIONALDICTIONARY_H
#define STINGRAYKIT_COLLECTION_TRANSACTIONALDICTIONARY_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableBuilder.h>
#include <stingraykit/collection/ITransactionalDictionary.h>
#include <stingraykit/collection/MapDictionary.h>
#include <stingraykit/signal/signals.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename KeyType_, typename ValueType_, typename KeyLessComparer_ = comparers::Less, typename ValueEqualsComparer_ = comparers::Equals >
	class TransactionalDictionary : public virtual ITransactionalDictionary<KeyType_, ValueType_>
	{
		typedef signal_policies::threading::ExternalMutexPointer ExternalMutexPointer;

	public:
		typedef ITransactionalDictionary<KeyType_, ValueType_>	base;

		typedef typename base::KeyType							KeyType;
		typedef typename base::ValueType						ValueType;
		typedef typename base::PairType							PairType;

		typedef typename base::DiffEntryType					DiffEntryType;
		typedef typename base::DiffTypePtr						DiffTypePtr;
		typedef typename base::TransactionTypePtr				TransactionTypePtr;

		typedef KeyLessComparer_								KeyLessComparer;
		typedef ValueEqualsComparer_							ValueEqualsComparer;

		typedef MapDictionary<KeyType, ValueType, KeyLessComparer>	 DictionaryImpl;
		STINGRAYKIT_DECLARE_PTR(DictionaryImpl);

	private:
		class Impl
		{
		private:
			shared_ptr<Mutex>											_mutex;
			DictionaryImplPtr											_map;
			bool														_hasTransaction;
			signal<void (const DiffTypePtr&), ExternalMutexPointer>		_onChanged;

		public:
			Impl()
				:	_mutex(make_shared<Mutex>()),
					_map(make_shared<DictionaryImpl>()),
					_hasTransaction(false),
					_onChanged(ExternalMutexPointer(_mutex), bind(&Impl::OnChangedPopulator, this, _1))
			{ }

			shared_ptr<IEnumerator<PairType> > GetEnumerator() const
			{
				MutexLock l(*_mutex);
				return _map->GetEnumerator();
			}

			shared_ptr<IEnumerable<PairType> > Reverse() const
			{
				MutexLock l(*_mutex);
				return _map->Reverse();
			}

			size_t GetCount() const
			{
				MutexLock l(*_mutex);
				return _map->GetCount();
			}

			bool IsEmpty() const
			{
				MutexLock l(*_mutex);
				return _map->IsEmpty();
			}

			bool ContainsKey(const KeyType& key) const
			{
				MutexLock l(*_mutex);
				return _map->ContainsKey(key);
			}

			shared_ptr<IEnumerator<PairType> > Find(const KeyType& key) const
			{
				MutexLock l(*_mutex);
				return _map->Find(key);
			}

			shared_ptr<IEnumerator<PairType> > ReverseFind(const KeyType& key) const
			{
				MutexLock l(*_mutex);
				return _map->ReverseFind(key);
			}

			ValueType Get(const KeyType& key) const
			{
				MutexLock l(*_mutex);
				return _map->Get(key);
			}

			bool TryGet(const KeyType& key, ValueType& outValue) const
			{
				MutexLock l(*_mutex);
				return _map->TryGet(key, outValue);
			}

			void BeginTransaction()
			{
				MutexLock l(*_mutex);
				STINGRAYKIT_CHECK(!_hasTransaction, "Another transaction exist!");
				_hasTransaction = true;
			}

			void EndTransaction()
			{
				MutexLock l(*_mutex);
				_hasTransaction = false;
			}

			DictionaryImplPtr GetCopy() const
			{
				MutexLock l(*_mutex);
				return make_shared<DictionaryImpl>(*_map);
			}

			void Apply(const DictionaryImplPtr& newMap)
			{
				STINGRAYKIT_CHECK(newMap, NullArgumentException("newMap"));

				MutexLock l(*_mutex);
				STINGRAYKIT_CHECK(_hasTransaction, InvalidOperationException("No transaction"));

				const DiffTypePtr diff = Diff(newMap);
				_map = newMap;
				_onChanged(diff);
			}

			DiffTypePtr Diff(const DictionaryImplPtr& newMap)
			{
				STINGRAYKIT_CHECK(newMap, NullArgumentException("newMap"));

				typedef std::vector<DiffEntryType> OutDiffContainer;
				shared_ptr<OutDiffContainer> diff = make_shared<OutDiffContainer>();

				MutexLock l(*_mutex);
				shared_ptr<IEnumerator<PairType> > old = _map->GetEnumerator();
				shared_ptr<IEnumerator<PairType> > copy = newMap->GetEnumerator();
				while (old->Valid() || copy->Valid())
				{
					if (!copy->Valid())
					{
						diff->push_back(MakeDiffEntry(CollectionOp::Removed, old->Get()));
						old->Next();
					}
					else if (!old->Valid() || KeyLessComparer()(copy->Get().Key, old->Get().Key))
					{
						diff->push_back(MakeDiffEntry(CollectionOp::Added, copy->Get()));
						copy->Next();
					}
					else if (KeyLessComparer()(old->Get().Key, copy->Get().Key))
					{
						diff->push_back(MakeDiffEntry(CollectionOp::Removed, old->Get()));
						old->Next();
					}
					else // old->Get().Key == copy->Get().Key
					{
						if (!ValueEqualsComparer()(old->Get().Value, copy->Get().Value))
						{
							// TODO: fix Updated handling
							//diff->push_back(MakeDiffEntry(CollectionOp::Updated, copy->Get()));
							diff->push_back(MakeDiffEntry(CollectionOp::Removed, old->Get()));
							diff->push_back(MakeDiffEntry(CollectionOp::Added, copy->Get()));
						}
						old->Next();
						copy->Next();
					}
				}
				return EnumerableFromStlContainer(*diff, diff);
			}

			signal_connector<void (const DiffTypePtr&)> OnChanged() const
			{ return _onChanged.connector(); }

			const Mutex& GetSyncRoot() const
			{ return *_mutex; }

		private:
			void OnChangedPopulator(const function<void (const DiffTypePtr&)>& slot) const
			{ slot(WrapEnumerable(_map, bind(&MakeDiffEntry<PairType>, CollectionOp::Added, _1))); }
		};
		STINGRAYKIT_DECLARE_PTR(Impl);

	private:
		class Transaction : public virtual IDictionaryTransaction<KeyType, ValueType>
		{
		private:
			ImplPtr							_impl;
			mutable DictionaryImplPtr		_newMap;

		public:
			explicit Transaction(const ImplPtr& impl)
				:	_impl(STINGRAYKIT_REQUIRE_NOT_NULL(impl))
			{ _impl->BeginTransaction(); }

			virtual ~Transaction()
			{ _impl->EndTransaction(); }

			virtual shared_ptr<IEnumerator<PairType> > GetEnumerator() const
			{ return GetCopy()->GetEnumerator(); }

			virtual shared_ptr<IEnumerable<PairType> > Reverse() const
			{ return GetCopy()->Reverse(); }

			virtual size_t GetCount() const
			{ return GetCopy()->GetCount(); }

			virtual bool IsEmpty() const
			{ return GetCopy()->IsEmpty(); }

			virtual bool ContainsKey(const KeyType& key) const
			{ return GetCopy()->ContainsKey(key); }

			virtual shared_ptr<IEnumerator<PairType> > Find(const KeyType& key) const
			{ return GetCopy()->Find(key); }

			virtual shared_ptr<IEnumerator<PairType> > ReverseFind(const KeyType& key) const
			{ return GetCopy()->ReverseFind(key); }

			virtual ValueType Get(const KeyType& key) const
			{ return GetCopy()->Get(key); }

			virtual bool TryGet(const KeyType& key, ValueType& outValue) const
			{ return GetCopy()->TryGet(key, outValue); }

			virtual void Set(const KeyType& key, const ValueType& value)
			{ GetCopy()->Set(key, value); }

			virtual void Remove(const KeyType& key)
			{ GetCopy()->Remove(key); }

			virtual bool TryRemove(const KeyType& key)
			{ return GetCopy()->TryRemove(key); }

			virtual size_t RemoveWhere(const function<bool (const KeyType&, const ValueType&)>& pred)
			{ return GetCopy()->RemoveWhere(pred); }

			virtual void Clear()
			{ GetCopy()->Clear(); }

			virtual void Apply(const DiffEntryType& entry)
			{
				switch (entry.Op)
				{
				case CollectionOp::Added:
					Set(entry.Item.Key, entry.Item.Value);
					break;
				case CollectionOp::Updated:
					Set(entry.Item.Key, entry.Item.Value);
					break;
				case CollectionOp::Removed:
					Remove(entry.Item.Key);
					break;
				}
			}

			virtual void Commit()
			{
				if (_newMap)
					_impl->Apply(_newMap);

				_newMap.reset();
			}

			virtual void Revert()
			{ _newMap.reset(); }

			virtual DiffTypePtr Diff() const
			{ return _newMap ? _impl->Diff(_newMap) : MakeEmptyEnumerable(); }

		private:
			DictionaryImplPtr GetCopy() const
			{ return _newMap ? _newMap : _newMap = _impl->GetCopy(); }
		};
		STINGRAYKIT_DECLARE_PTR(Transaction);

	private:
		ImplPtr							_impl;

	public:
		TransactionalDictionary()
			:	_impl(make_shared<Impl>())
		{ }

		virtual shared_ptr<IEnumerator<PairType> > GetEnumerator() const
		{ return _impl->GetEnumerator(); }

		virtual shared_ptr<IEnumerable<PairType> > Reverse() const
		{ return _impl->Reverse(); }

		virtual size_t GetCount() const
		{ return _impl->GetCount(); }

		virtual bool IsEmpty() const
		{ return _impl->IsEmpty(); }

		virtual bool ContainsKey(const KeyType& key) const
		{ return _impl->ContainsKey(key); }

		virtual shared_ptr<IEnumerator<PairType> > Find(const KeyType& key) const
		{ return _impl->Find(key); }

		virtual shared_ptr<IEnumerator<PairType> > ReverseFind(const KeyType& key) const
		{ return _impl->ReverseFind(key); }

		virtual ValueType Get(const KeyType& key) const
		{ return _impl->Get(key); }

		virtual bool TryGet(const KeyType& key, ValueType& outValue) const
		{ return _impl->TryGet(key, outValue); }

		virtual TransactionTypePtr StartTransaction()
		{ return make_shared<Transaction>(_impl); }

		virtual signal_connector<void (const DiffTypePtr&)> OnChanged() const
		{ return _impl->OnChanged(); }

		virtual const Mutex& GetSyncRoot() const
		{ return _impl->GetSyncRoot(); }

		ObservableCollectionLockerPtr Lock() const
		{ return make_shared<ObservableCollectionLocker>(*this); }
	};

	/** @} */

}

#endif
