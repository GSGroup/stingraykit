#ifndef STINGRAYKIT_COLLECTION_TRANSACTIONALDICTIONARYWRAPPER_H
#define STINGRAYKIT_COLLECTION_TRANSACTIONALDICTIONARYWRAPPER_H

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
	class DictionaryTransactionImpl : public virtual IDictionaryTransaction<KeyType_, ValueType_>
	{
		typedef signal_policies::threading::ExternalMutexPointer ExternalMutexPointer;

	public:
		typedef IDictionaryTransaction<KeyType_, ValueType_>	base;

		typedef typename base::KeyType							KeyType;
		typedef typename base::ValueType						ValueType;
		typedef typename base::PairType							PairType;

		typedef typename base::DiffEntryType					DiffEntryType;
		typedef typename base::DiffTypePtr						DiffTypePtr;

		typedef KeyLessComparer_								KeyLessComparer;
		typedef ValueEqualsComparer_							ValueEqualsComparer;

		typedef MapDictionary<KeyType, ValueType, KeyLessComparer>	 DictionaryImpl;
		STINGRAYKIT_DECLARE_PTR(DictionaryImpl);

		typedef signal<void(const DiffTypePtr&), ExternalMutexPointer> OnChangedSignalType;

	private:
		DictionaryImplPtr&				_wrapped;
		mutable DictionaryImplPtr		_copy;
		const OnChangedSignalType&		_onChanged;
		ValueEqualsComparer				_comparer;

	public:
		DictionaryTransactionImpl(DictionaryImplPtr& wrapped, const OnChangedSignalType& onChanged)
			:	_wrapped(wrapped),
				_onChanged(onChanged)
		{ }

		virtual ~DictionaryTransactionImpl()
		{
			if (_copy)
				Logger::Warning() << "Reverting DictionaryTransactionImpl!";
		}

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
			if (!_copy)
				return;

			signal_locker l(_onChanged);
			const DiffTypePtr diff = Diff();
			_wrapped = _copy;
			_copy.reset();
			_onChanged(diff);
		}

		virtual void Revert()
		{ _copy.reset(); }

		virtual DiffTypePtr Diff() const
		{
			if (!_copy)
				return MakeEmptyEnumerable();

			typedef std::vector<DiffEntryType> OutDiffContainer;
			shared_ptr<OutDiffContainer> diff = make_shared<OutDiffContainer>();

			signal_locker l(_onChanged);
			shared_ptr<IEnumerator<PairType> > old = _wrapped->GetEnumerator();
			shared_ptr<IEnumerator<PairType> > copy = _copy->GetEnumerator();
			while (old->Valid() || copy->Valid())
			{
				if (!copy->Valid())
				{
					diff->push_back(MakeDiffEntry(CollectionOp::Removed, old->Get()));
					old->Next();
				}
				else if (!old->Valid() || old->Get().Key > copy->Get().Key)
				{
					diff->push_back(MakeDiffEntry(CollectionOp::Added, copy->Get()));
					copy->Next();
				}
				else if (old->Get().Key < copy->Get().Key)
				{
					diff->push_back(MakeDiffEntry(CollectionOp::Removed, old->Get()));
					old->Next();
				}
				else // old->Get().Key == copy->Get().Key
				{
					if (!_comparer(old->Get().Value, copy->Get().Value))
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

	private:
		DictionaryImplPtr GetCopy() const
		{
			if (!_copy)
			{
				signal_locker l(_onChanged);
				_copy = make_shared<DictionaryImpl>(*_wrapped);
			}
			return _copy;
		}
	};


	template < typename KeyType_, typename ValueType_, typename KeyLessComparer_ = comparers::Less, typename ValueEqualsComparer_ = comparers::Equals >
	class TransactionalDictionaryWrapper : public virtual ITransactionalDictionary<KeyType_, ValueType_>
	{
		typedef signal_policies::threading::ExternalMutexPointer ExternalMutexPointer;

	public:
		typedef ITransactionalDictionary<KeyType_, ValueType_>	base;

		typedef typename base::KeyType							KeyType;
		typedef typename base::ValueType						ValueType;
		typedef typename base::PairType							PairType;

		typedef typename base::DiffEntryType					DiffEntryType;
		typedef typename base::DiffTypePtr						DiffTypePtr;

		typedef KeyLessComparer_								KeyLessComparer;
		typedef ValueEqualsComparer_							ValueEqualsComparer;

		typedef MapDictionary<KeyType, ValueType, KeyLessComparer>	 DictionaryImpl;
		STINGRAYKIT_DECLARE_PTR(DictionaryImpl);

	private:
		shared_ptr<Mutex>											_mutex;
		DictionaryImplPtr											_wrapped;
		typename base::TransactionTypeWeakPtr						_transaction;
		signal<void (const DiffTypePtr&), ExternalMutexPointer>		_onChanged;

	public:
		TransactionalDictionaryWrapper()
			:	_mutex(make_shared<Mutex>()),
				_wrapped(make_shared<DictionaryImpl>()),
				_onChanged(ExternalMutexPointer(_mutex), bind(&TransactionalDictionaryWrapper::OnChangedPopulator, this, _1))
		{ }

		virtual shared_ptr<IEnumerator<PairType> > GetEnumerator() const
		{
			signal_locker l(_onChanged);
			return _wrapped->GetEnumerator();
		}

		virtual shared_ptr<IEnumerable<PairType> > Reverse() const
		{
			signal_locker l(_onChanged);
			return _wrapped->Reverse();
		}

		virtual size_t GetCount() const
		{
			signal_locker l(_onChanged);
			return _wrapped->GetCount();
		}

		virtual bool IsEmpty() const
		{
			signal_locker l(_onChanged);
			return _wrapped->IsEmpty();
		}

		virtual bool ContainsKey(const KeyType& key) const
		{
			signal_locker l(_onChanged);
			return _wrapped->ContainsKey(key);
		}

		virtual shared_ptr<IEnumerator<PairType> > Find(const KeyType& key) const
		{
			signal_locker l(_onChanged);
			return _wrapped->Find(key);
		}

		virtual shared_ptr<IEnumerator<PairType> > ReverseFind(const KeyType& key) const
		{
			signal_locker l(_onChanged);
			return _wrapped->ReverseFind(key);
		}

		virtual ValueType Get(const KeyType& key) const
		{
			signal_locker l(_onChanged);
			return _wrapped->Get(key);
		}

		virtual bool TryGet(const KeyType& key, ValueType& outValue) const
		{
			signal_locker l(_onChanged);
			return _wrapped->TryGet(key, outValue);
		}

		virtual typename base::TransactionTypePtr StartTransaction()
		{
			signal_locker l(_onChanged);
			STINGRAYKIT_CHECK(_transaction.expired(), "Another transaction exist!");
			typename base::TransactionTypePtr tr = make_shared<DictionaryTransactionImpl<KeyType, ValueType, KeyLessComparer, ValueEqualsComparer> >(ref(_wrapped), _onChanged);
			_transaction = tr;
			return tr;
		}

		virtual signal_connector<void (const DiffTypePtr&)> OnChanged() const
		{ return _onChanged.connector(); }

		virtual const Mutex& GetSyncRoot() const
		{ return *_mutex; }

		ObservableCollectionLockerPtr Lock() const
		{ return make_shared<ObservableCollectionLocker>(*this); }

	private:
		void OnChangedPopulator(const function<void (const DiffTypePtr&)>& slot) const
		{
			EnumerableBuilder<DiffEntryType> diff;
			FOR_EACH(PairType p IN _wrapped->GetEnumerator())
				diff.Add(DiffEntryType(CollectionOp::Added, p));
			slot(diff.Get());
		}
	};

	/** @} */

}

#endif
