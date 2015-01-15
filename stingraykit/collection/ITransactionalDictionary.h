#ifndef STINGRAYKIT_COLLECTION_ITRANSACTIONALDICTIONARY_H
#define STINGRAYKIT_COLLECTION_ITRANSACTIONALDICTIONARY_H

#include <stingraykit/collection/DiffEntry.h>
#include <stingraykit/collection/IDictionary.h>
#include <stingraykit/collection/ObservableCollectionLocker.h>
#include <stingraykit/signal/signals.h>
#include <stingraykit/toolkit.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename KeyType_, typename ValueType_ >
	struct IDictionaryTransaction : public virtual IDictionary<KeyType_, ValueType_>
	{
		typedef IDictionary<KeyType_, ValueType_>			base;
		typedef DiffEntry<typename base::PairType>			DiffEntryType;
		typedef IEnumerable<DiffEntryType>					DiffType;
		STINGRAYKIT_DECLARE_PTR(DiffType);

		virtual DiffTypePtr Diff() const = 0;

		void Apply(const DiffEntryType& entry)
		{
			switch (entry.Op)
			{
			case CollectionOp::Added:
				this->Set(entry.Item.Key, entry.Item.Value);
				break;
			case CollectionOp::Updated:
				this->Set(entry.Item.Key, entry.Item.Value);
				break;
			case CollectionOp::Removed:
				this->Remove(entry.Item.Key); // gcc loses his mind over this line without this
				break;
			default:
				STINGRAYKIT_THROW("Not supported CollectionOp");
			}
		}

		virtual void Commit() = 0;
		virtual void Revert() = 0;
	};


	template < typename KeyType_, typename ValueType_ >
	struct ITransactionalDictionary :
		public virtual IReadonlyDictionary<KeyType_, ValueType_>
	{
		typedef IReadonlyDictionary<KeyType_, ValueType_>	base;
		typedef typename base::PairType						PairType;

		typedef DiffEntry<PairType>							DiffEntryType;
		typedef IEnumerable<DiffEntryType>					DiffType;
		STINGRAYKIT_DECLARE_PTR(DiffType);

		typedef IDictionaryTransaction<KeyType_, ValueType_> TransactionType;
		STINGRAYKIT_DECLARE_PTR(TransactionType);

		virtual const Mutex& GetSyncRoot() const = 0;
		virtual signal_connector<void(const DiffTypePtr&)> OnChanged() const = 0;

		ObservableCollectionLockerPtr Lock() const { return make_shared<ObservableCollectionLocker>(*this); }

		virtual TransactionTypePtr StartTransaction() = 0;

	protected:
		ITransactionalDictionary()
		{ }
	};

	/** @} */

}


#endif

