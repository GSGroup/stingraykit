#ifndef STINGRAY_TOOLKIT_ITRANSACTIONALDICTIONARY_H
#define STINGRAY_TOOLKIT_ITRANSACTIONALDICTIONARY_H

#include <stingray/toolkit/DiffEntry.h>
#include <stingray/toolkit/IDictionary.h>
#include <stingray/toolkit/ObservableCollectionLocker.h>
#include <stingray/toolkit/signals.h>
#include <stingray/toolkit/toolkit.h>

namespace stingray
{


	namespace Detail
	{
		template < typename T, typename KeyType_, typename ValueType_, bool IsSerializable_ = IsSerializable<KeyType_>::Value && IsSerializable<ValueType_>::Value >
		struct SerializableTransactionalDictionary : public virtual ISerializable
		{
			virtual ~SerializableTransactionalDictionary() { }

			virtual void Serialize(ObjectOStream & ar) const
			{
				typedef KeyValuePair<KeyType_, ValueType_>	PairType;
				const T* inst = static_cast<const T*>(this);
				std::map<KeyType_, ValueType_> m;
				FOR_EACH(PairType p IN inst->GetEnumerator())
					m.insert(std::make_pair(p.Key, p.Value));
				ar.Serialize("data", m);
			}

			virtual void Deserialize(ObjectIStream & ar)
			{
				T* inst = static_cast<T*>(this);
				typename T::TransactionTypePtr trans = inst->StartTransaction();
				std::map<KeyType_, ValueType_> m;
				ar.Deserialize("data", m);
				trans->Clear();
				for (typename std::map<KeyType_, ValueType_>::const_iterator it = m.begin(); it != m.end(); ++it)
					trans->Set(it->first, it->second);
				trans->Commit();
			}
		};

		template < typename T, typename KeyType_, typename ValueType_ >
		struct SerializableTransactionalDictionary<T, KeyType_, ValueType_, false>
		{
			virtual ~SerializableTransactionalDictionary() { }
		};
	}


	template < typename KeyType_, typename ValueType_ >
	struct IDictionaryTransaction : public virtual IDictionary<KeyType_, ValueType_>
	{
		typedef IDictionary<KeyType_, ValueType_>			base;
		typedef DiffEntry<typename base::PairType>			DiffEntryType;
		typedef IEnumerable<DiffEntryType>					DiffType;
		TOOLKIT_DECLARE_PTR(DiffType);

		virtual void Commit() = 0;
		virtual void Revert() = 0;
	};


	template < typename KeyType_, typename ValueType_ >
	struct ITransactionalDictionary :
		public virtual IReadonlyDictionary<KeyType_, ValueType_>,
		public Detail::SerializableTransactionalDictionary<ITransactionalDictionary<KeyType_, ValueType_>, KeyType_, ValueType_ >
	{
		typedef IReadonlyDictionary<KeyType_, ValueType_>	base;
		typedef typename base::PairType						PairType;

		typedef DiffEntry<PairType>							DiffEntryType;
		typedef IEnumerable<DiffEntryType>					DiffType;
		TOOLKIT_DECLARE_PTR(DiffType);

		typedef IDictionaryTransaction<KeyType_, ValueType_> TransactionType;
		TOOLKIT_DECLARE_PTR(TransactionType);

		signal<void(const DiffTypePtr&)>					OnChanged;

		ObservableCollectionLockerPtr Lock() const { return make_shared<ObservableCollectionLocker>(*this); }

		virtual TransactionTypePtr StartTransaction() = 0;

	protected:
		ITransactionalDictionary()
		{ }
	};


	template < typename Wrapped_>
	class DictionaryTransactionImpl : public virtual IDictionaryTransaction<typename Wrapped_::KeyType, typename Wrapped_::ValueType>
	{
		typedef IDictionaryTransaction<typename Wrapped_::KeyType, typename Wrapped_::ValueType> base;
		typedef typename base::KeyType					KeyType;
		typedef typename base::ValueType				ValueType;
		typedef typename base::PairType					PairType;
		typedef typename base::DiffEntryType			DiffEntryType;
		typedef typename base::DiffTypePtr				DiffTypePtr;

		typedef shared_ptr<Wrapped_>					WrappedPtr;

		typedef signal<void(const DiffTypePtr&)>		OnChangedSignalType;

	private:
		WrappedPtr						&_wrapped;
		mutable WrappedPtr				_copy;
		const OnChangedSignalType		&_onChanged;

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

		virtual void Commit()
		{
			if (!_copy)
				return;

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
					if (old->Get().Value != copy->Get().Value)
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
			if (diff->size())
				_onChanged(EnumerableFromStlContainer(*diff, diff));
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


	template < typename Wrapped_ >
	class TransactionalDictionaryWrapper :
		public virtual ITransactionalDictionary<typename Wrapped_::KeyType, typename Wrapped_::ValueType>
	{
	public:
		typedef typename Wrapped_::KeyType						KeyType;
		typedef typename Wrapped_::ValueType					ValueType;
		typedef typename Wrapped_::PairType						PairType;
		typedef ITransactionalDictionary<KeyType, ValueType>	TransactionalInterface;

	private:
		shared_ptr<Wrapped_>									_wrapped;
		typename TransactionalInterface::TransactionTypeWeakPtr	_transaction;

	public:
		TransactionalDictionaryWrapper() : _wrapped(new Wrapped_())
		{}

		virtual int GetCount() const
		{
			signal_locker l(TransactionalInterface::OnChanged);
			return _wrapped->GetCount();
		}

		virtual bool IsEmpty() const
		{
			signal_locker l(TransactionalInterface::OnChanged);
			return _wrapped->IsEmpty();
		}

		virtual ValueType Get(const KeyType& key) const
		{
			signal_locker l(TransactionalInterface::OnChanged);
			return _wrapped->Get(key);
		}

		virtual bool ContainsKey(const KeyType& key) const
		{
			signal_locker l(TransactionalInterface::OnChanged);
			return _wrapped->ContainsKey(key);
		}

		virtual bool TryGet(const KeyType& key, ValueType& outValue) const
		{
			signal_locker l(TransactionalInterface::OnChanged);
			return _wrapped->TryGet(key, outValue);
		}

		virtual shared_ptr<IEnumerator<PairType> > GetEnumerator() const
		{
			signal_locker l(TransactionalInterface::OnChanged);
			return _wrapped->GetEnumerator();
		}

		virtual shared_ptr<IEnumerable<PairType> > Reverse() const
		{
			signal_locker l(TransactionalInterface::OnChanged);
			return _wrapped->Reverse();
		}

		ObservableCollectionLockerPtr Lock() const { return make_shared<ObservableCollectionLocker>(*this); }

		virtual typename TransactionalInterface::TransactionTypePtr StartTransaction()
		{
			signal_locker l(TransactionalInterface::OnChanged);
			TOOLKIT_CHECK(!_transaction.lock(), "Another transaction exist!");
			typename TransactionalInterface::TransactionTypePtr tr(new DictionaryTransactionImpl<Wrapped_>(_wrapped, TransactionalInterface::OnChanged));
			_transaction = tr;
			return tr;
		}
	};



}


#endif

