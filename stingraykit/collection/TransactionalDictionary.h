#ifndef STINGRAYKIT_COLLECTION_TRANSACTIONALDICTIONARY_H
#define STINGRAYKIT_COLLECTION_TRANSACTIONALDICTIONARY_H

// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableBuilder.h>
#include <stingraykit/collection/ITransactionalDictionary.h>
#include <stingraykit/collection/MapDictionary.h>
#include <stingraykit/diagnostics/ExecutorsProfiler.h>
#include <stingraykit/log/Logger.h>
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
		struct Utils
		{
			static DiffTypePtr Diff(const DictionaryImplPtr& oldMap, const DictionaryImplPtr& newMap)
			{
				STINGRAYKIT_CHECK(oldMap, NullArgumentException("oldMap"));
				STINGRAYKIT_CHECK(newMap, NullArgumentException("newMap"));

				const shared_ptr<std::vector<DiffEntryType> > diff = make_shared<std::vector<DiffEntryType> >();

				const shared_ptr<IEnumerator<PairType> > oldEn = oldMap->GetEnumerator();
				const shared_ptr<IEnumerator<PairType> > newEn = newMap->GetEnumerator();

				while (oldEn->Valid() || newEn->Valid())
				{
					if (!newEn->Valid())
					{
						diff->push_back(MakeDiffEntry(CollectionOp::Removed, oldEn->Get()));
						oldEn->Next();
					}
					else if (!oldEn->Valid() || KeyLessComparer()(newEn->Get().Key, oldEn->Get().Key))
					{
						diff->push_back(MakeDiffEntry(CollectionOp::Added, newEn->Get()));
						newEn->Next();
					}
					else if (KeyLessComparer()(oldEn->Get().Key, newEn->Get().Key))
					{
						diff->push_back(MakeDiffEntry(CollectionOp::Removed, oldEn->Get()));
						oldEn->Next();
					}
					else // oldEn->Get().Key == newEn->Get().Key
					{
						if (!ValueEqualsComparer()(oldEn->Get().Value, newEn->Get().Value))
						{
							// TODO: fix Updated handling
							//diff->push_back(MakeDiffEntry(CollectionOp::Updated, newEn->Get()));
							diff->push_back(MakeDiffEntry(CollectionOp::Removed, oldEn->Get()));
							diff->push_back(MakeDiffEntry(CollectionOp::Added, newEn->Get()));
						}
						oldEn->Next();
						newEn->Next();
					}
				}

				return EnumerableFromStlContainer(*diff, diff);
			}
		};

	private:
		struct ImplData
		{
			shared_ptr<Mutex>											Guard;
			DictionaryImplPtr											Map;
			bool														HasTransaction;
			ConditionVariable											TransactionCompleted;
			signal<void (const DiffTypePtr&), ExternalMutexPointer>		OnChanged;

		public:
			ImplData()
				:	Guard(make_shared<Mutex>()),
					Map(make_shared<DictionaryImpl>()),
					HasTransaction(false),
					OnChanged(ExternalMutexPointer(Guard), bind(&ImplData::OnChangedPopulator, this, _1))
			{ }

			DictionaryImplPtr BeginTransaction(const ICancellationToken& token)
			{
				optional<AsyncProfiler::Session> profilerSession;

				MutexLock l(*Guard);

				while (HasTransaction)
				{
					if (!profilerSession)
						profilerSession.emplace(ExecutorsProfiler::Instance().GetProfiler(), &GetProfilerMessage, TimeDuration::Second(), AsyncProfiler::NameGetterTag());

					switch (TransactionCompleted.Wait(*Guard, token))
					{
					case ConditionWaitResult::Broadcasted:	continue;
					case ConditionWaitResult::Cancelled:	STINGRAYKIT_THROW(OperationCancelledException());
					case ConditionWaitResult::TimedOut:		STINGRAYKIT_THROW(TimeoutException());
					}
				}

				HasTransaction = true;
				return Map;
			}

			void EndTransaction()
			{
				MutexLock l(*Guard);
				HasTransaction = false;
				TransactionCompleted.Broadcast();
			}

			void Apply(const DictionaryImplPtr& newMap, const DiffTypePtr& diff)
			{
				STINGRAYKIT_CHECK(newMap, NullArgumentException("newMap"));
				STINGRAYKIT_CHECK(diff, NullArgumentException("diff"));

				MutexLock l(*Guard);
				STINGRAYKIT_CHECK(HasTransaction, InvalidOperationException("No transaction"));

				Map = newMap;
				OnChanged(diff);
			}

		private:
			void OnChangedPopulator(const function<void (const DiffTypePtr&)>& slot) const
			{ slot(WrapEnumerable(Map, bind(&MakeDiffEntry<PairType>, CollectionOp::Added, _1))); }

			static std::string GetProfilerMessage()
			{ return StringBuilder() % "Starting transaction on " % TypeInfo(typeid(TransactionalDictionary)).GetClassName(); }
		};
		STINGRAYKIT_DECLARE_PTR(ImplData);

	private:
		class Transaction : public virtual IDictionaryTransaction<KeyType, ValueType>
		{
		private:
			ImplDataPtr						_impl;
			DictionaryImplPtr				_oldMap;
			DictionaryImplPtr				_newMap;
			mutable DiffTypePtr				_cachedDiff;

		public:
			Transaction(const ImplDataPtr& impl, const ICancellationToken& token)
				:	_impl(STINGRAYKIT_REQUIRE_NOT_NULL(impl)),
					_oldMap(_impl->BeginTransaction(token))
			{ }

			virtual ~Transaction()
			{ STINGRAYKIT_TRY_NO_MESSAGE(_impl->EndTransaction()); }

			virtual shared_ptr<IEnumerator<PairType> > GetEnumerator() const
			{ return _newMap ? _newMap->GetEnumerator() : _oldMap->GetEnumerator(); }

			virtual shared_ptr<IEnumerable<PairType> > Reverse() const
			{ return _newMap ? _newMap->Reverse() : _oldMap->Reverse(); }

			virtual size_t GetCount() const
			{ return _newMap ? _newMap->GetCount() : _oldMap->GetCount(); }

			virtual bool IsEmpty() const
			{ return _newMap ? _newMap->IsEmpty() : _oldMap->IsEmpty(); }

			virtual bool ContainsKey(const KeyType& key) const
			{ return _newMap ? _newMap->ContainsKey(key) : _oldMap->ContainsKey(key); }

			virtual shared_ptr<IEnumerator<PairType> > Find(const KeyType& key) const
			{ return _newMap ? _newMap->Find(key) : _oldMap->Find(key); }

			virtual shared_ptr<IEnumerator<PairType> > ReverseFind(const KeyType& key) const
			{ return _newMap ? _newMap->ReverseFind(key) : _oldMap->ReverseFind(key); }

			virtual ValueType Get(const KeyType& key) const
			{ return _newMap ? _newMap->Get(key) : _oldMap->Get(key); }

			virtual bool TryGet(const KeyType& key, ValueType& outValue) const
			{ return _newMap ? _newMap->TryGet(key, outValue) : _oldMap->TryGet(key, outValue); }

			virtual void Set(const KeyType& key, const ValueType& value)
			{ CopyOnWrite(); _newMap->Set(key, value); }

			virtual void Remove(const KeyType& key)
			{ CopyOnWrite(); _newMap->Remove(key); }

			virtual bool TryRemove(const KeyType& key)
			{ CopyOnWrite(); return _newMap->TryRemove(key); }

			virtual size_t RemoveWhere(const function<bool (const KeyType&, const ValueType&)>& pred)
			{ CopyOnWrite(); return _newMap->RemoveWhere(pred); }

			virtual void Clear()
			{
				_cachedDiff.reset();

				if (_newMap)
					_newMap->Clear();
				else
					_newMap = make_shared<DictionaryImpl>();
			}

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
					_impl->Apply(_newMap, _cachedDiff ? _cachedDiff : Utils::Diff(_oldMap, _newMap));

				_cachedDiff.reset();
				_newMap.reset();
			}

			virtual void Revert()
			{
				_cachedDiff.reset();
				_newMap.reset();
			}

			virtual DiffTypePtr Diff() const
			{
				if (_cachedDiff)
					return _cachedDiff;

				return _newMap ? _cachedDiff = Utils::Diff(_oldMap, _newMap) : MakeEmptyEnumerable();
			}

		private:
			void CopyOnWrite()
			{
				_cachedDiff.reset();

				if (!_newMap)
					_newMap = make_shared<DictionaryImpl>(*_oldMap);
			}
		};
		STINGRAYKIT_DECLARE_PTR(Transaction);

	private:
		ImplDataPtr							_impl;

	public:
		TransactionalDictionary()
			:	_impl(make_shared<ImplData>())
		{ }

		virtual shared_ptr<IEnumerator<PairType> > GetEnumerator() const
		{
			MutexLock l(*_impl->Guard);
			return _impl->Map->GetEnumerator();
		}

		virtual shared_ptr<IEnumerable<PairType> > Reverse() const
		{
			MutexLock l(*_impl->Guard);
			return _impl->Map->Reverse();
		}

		virtual size_t GetCount() const
		{
			MutexLock l(*_impl->Guard);
			return _impl->Map->GetCount();
		}

		virtual bool IsEmpty() const
		{
			MutexLock l(*_impl->Guard);
			return _impl->Map->IsEmpty();
		}

		virtual bool ContainsKey(const KeyType& key) const
		{
			MutexLock l(*_impl->Guard);
			return _impl->Map->ContainsKey(key);
		}

		virtual shared_ptr<IEnumerator<PairType> > Find(const KeyType& key) const
		{
			MutexLock l(*_impl->Guard);
			return _impl->Map->Find(key);
		}

		virtual shared_ptr<IEnumerator<PairType> > ReverseFind(const KeyType& key) const
		{
			MutexLock l(*_impl->Guard);
			return _impl->Map->ReverseFind(key);
		}

		virtual ValueType Get(const KeyType& key) const
		{
			MutexLock l(*_impl->Guard);
			return _impl->Map->Get(key);
		}

		virtual bool TryGet(const KeyType& key, ValueType& outValue) const
		{
			MutexLock l(*_impl->Guard);
			return _impl->Map->TryGet(key, outValue);
		}

		virtual TransactionTypePtr StartTransaction(const ICancellationToken& token = DummyCancellationToken())
		{ return make_shared<Transaction>(_impl, token); }

		virtual signal_connector<void (const DiffTypePtr&)> OnChanged() const
		{ return _impl->OnChanged.connector(); }

		virtual const Mutex& GetSyncRoot() const
		{ return *_impl->Guard; }

		ObservableCollectionLockerPtr Lock() const
		{ return make_shared<ObservableCollectionLocker>(*this); }
	};

	/** @} */

}

#endif
