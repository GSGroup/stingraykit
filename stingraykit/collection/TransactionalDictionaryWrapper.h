#ifndef STINGRAYKIT_COLLECTION_TRANSACTIONALDICTIONARYWRAPPER_H
#define STINGRAYKIT_COLLECTION_TRANSACTIONALDICTIONARYWRAPPER_H

#include <stingraykit/collection/ArrayList.h>
#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/IList.h>
#include <stingraykit/collection/ITransactionalDictionary.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename Wrapped_, typename Comparer >
	class DictionaryTransactionImpl : public virtual IDictionaryTransaction<typename Wrapped_::KeyType, typename Wrapped_::ValueType>
	{
		typedef IDictionaryTransaction<typename Wrapped_::KeyType, typename Wrapped_::ValueType> base;
		typedef typename base::KeyType					KeyType;
		typedef typename base::ValueType				ValueType;
		typedef typename base::PairType					PairType;
		typedef typename base::DiffEntryType			DiffEntryType;
		typedef typename base::DiffTypePtr				DiffTypePtr;

		typedef shared_ptr<Wrapped_>					WrappedPtr;

		typedef signal<void(const DiffTypePtr&), signal_policies::threading::ExternalMutexPointer> OnChangedSignalType;

	private:
		WrappedPtr						&_wrapped;
		mutable WrappedPtr				_copy;
		const OnChangedSignalType		&_onChanged;
		Comparer						_comparer;

	public:
		DictionaryTransactionImpl(WrappedPtr &wrapped, const OnChangedSignalType& onChanged) :
			_wrapped(wrapped), _onChanged(onChanged)
		{ }

		virtual ~DictionaryTransactionImpl()
		{
			if (_copy)
				Logger::Warning() << "Reverting DictionaryTransactionImpl!";
		}

		virtual int GetCount() const
		{ return GetCopy()->GetCount(); }

		virtual bool IsEmpty() const
		{ return GetCopy()->IsEmpty(); }

		virtual bool ContainsKey(const KeyType& key) const
		{ return GetCopy()->ContainsKey(key); }

		virtual ValueType Get(const KeyType& key) const
		{ return GetCopy()->Get(key); }

		virtual void Set(const KeyType& key, const ValueType& value)
		{ GetCopy()->Set(key, value); }

		virtual void Remove(const KeyType& key)
		{ GetCopy()->Remove(key); }

		virtual bool TryGet(const KeyType& key, ValueType& outValue) const
		{ return GetCopy()->TryGet(key, outValue); }

		virtual bool TryRemove(const KeyType& key)
		{ return GetCopy()->TryRemove(key); }

		virtual void Clear()
		{ GetCopy()->Clear(); }

		virtual shared_ptr<IEnumerator<PairType> > GetEnumerator() const
		{ return GetCopy()->GetEnumerator(); }

		virtual shared_ptr<IEnumerable<PairType> > Reverse() const
		{ return GetCopy()->Reverse(); }

		virtual DiffTypePtr Diff() const
		{
			if (!_copy)
				return MakeEmptyEnumerable();

			typedef std::vector<DiffEntryType> OutDiffContainer;
			shared_ptr<OutDiffContainer> diff(new OutDiffContainer());

			signal_locker l(_onChanged);
			shared_ptr<IEnumerator<PairType> > old = _wrapped->GetEnumerator();
			shared_ptr<IEnumerator<PairType> > copy = _copy->GetEnumerator();
			while (old->Valid() || copy->Valid())
			{
				if (!copy->Valid())
				{
					diff->push_back(MakeDiffEntry(old->Get(), CollectionOp::Removed));
					old->Next();
				}
				else if (!old->Valid() || old->Get().Key > copy->Get().Key)
				{
					diff->push_back(MakeDiffEntry(copy->Get(), CollectionOp::Added));
					copy->Next();
				}
				else if (old->Get().Key < copy->Get().Key)
				{
					diff->push_back(MakeDiffEntry(old->Get(), CollectionOp::Removed));
					old->Next();
				}
				else // old->Get().Key == copy->Get().Key
				{
					if (!_comparer(old->Get().Value, copy->Get().Value))
					{
						// TODO: fix Updated handling
						//diff->push_back(MakeDiffEntry(copy->Get(), CollectionOp::Updated));
						diff->push_back(MakeDiffEntry(old->Get(), CollectionOp::Removed));
						diff->push_back(MakeDiffEntry(copy->Get(), CollectionOp::Added));
					}
					old->Next();
					copy->Next();
				}
			}
			return EnumerableFromStlContainer(*diff, diff);
		}

		virtual void Commit()
		{
			if (!_copy)
				return;

			signal_locker l(_onChanged);
			_onChanged(Diff());
			_wrapped = _copy;
			_copy.reset();
		}

		virtual void Revert()
		{ _copy.reset(); }

	private:
		WrappedPtr GetCopy() const
		{
			if (!_copy)
			{
				signal_locker l(_onChanged);
				_copy.reset(new Wrapped_(*_wrapped));
			}
			return _copy;
		}
	};


	template < typename Wrapped_, typename Comparer = std::equal_to<typename Wrapped_::ValueType> >
	class TransactionalDictionaryWrapper :
		public virtual ITransactionalDictionary<typename Wrapped_::KeyType, typename Wrapped_::ValueType>
	{
	public:
		typedef typename Wrapped_::KeyType						KeyType;
		typedef typename Wrapped_::ValueType					ValueType;
		typedef typename Wrapped_::PairType						PairType;
		typedef ITransactionalDictionary<KeyType, ValueType>	TransactionalInterface;
		typedef typename TransactionalInterface::DiffEntryType	DiffEntryType;
		typedef typename TransactionalInterface::DiffTypePtr	DiffTypePtr;

	private:
		shared_ptr<Mutex>																	_mutex;
		shared_ptr<Wrapped_>																_wrapped;
		typename TransactionalInterface::TransactionTypeWeakPtr								_transaction;
		signal<void(const DiffTypePtr&), signal_policies::threading::ExternalMutexPointer>	_onChanged;

	public:
		TransactionalDictionaryWrapper() :
			_mutex(new Mutex()), _wrapped(new Wrapped_()),
			_onChanged(signal_policies::threading::ExternalMutexPointer(_mutex), bind(&TransactionalDictionaryWrapper::OnChangedPopulator, this, _1))
		{}

		virtual const Mutex& GetSyncRoot() const
		{ return *_mutex; }

		virtual signal_connector<void(const DiffTypePtr&)> OnChanged() const
		{ return _onChanged.connector(); }

		virtual int GetCount() const
		{
			signal_locker l(_onChanged);
			return _wrapped->GetCount();
		}

		virtual bool IsEmpty() const
		{
			signal_locker l(_onChanged);
			return _wrapped->IsEmpty();
		}

		virtual ValueType Get(const KeyType& key) const
		{
			signal_locker l(_onChanged);
			return _wrapped->Get(key);
		}

		virtual bool ContainsKey(const KeyType& key) const
		{
			signal_locker l(_onChanged);
			return _wrapped->ContainsKey(key);
		}

		virtual bool TryGet(const KeyType& key, ValueType& outValue) const
		{
			signal_locker l(_onChanged);
			return _wrapped->TryGet(key, outValue);
		}

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

		ObservableCollectionLockerPtr Lock() const { return make_shared<ObservableCollectionLocker>(*this); }

		virtual typename TransactionalInterface::TransactionTypePtr StartTransaction()
		{
			signal_locker l(_onChanged);
			STINGRAYKIT_CHECK(!_transaction.lock(), "Another transaction exist!");
			typename TransactionalInterface::TransactionTypePtr tr(new DictionaryTransactionImpl<Wrapped_, Comparer>(_wrapped, _onChanged));
			_transaction = tr;
			return tr;
		}

	private:
		void OnChangedPopulator(const function<void(const DiffTypePtr&)>& slot) const
		{
			shared_ptr<IList<DiffEntryType> > diff(new ArrayList<DiffEntryType>);
			FOR_EACH(PairType p IN GetEnumerator())
				diff->Add(DiffEntryType(p, CollectionOp::Added));
			slot(diff);
		}
	};

	/** @} */

}

#endif
