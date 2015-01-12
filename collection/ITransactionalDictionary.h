#ifndef STINGRAY_TOOLKIT_COLLECTION_ITRANSACTIONALDICTIONARY_H
#define STINGRAY_TOOLKIT_COLLECTION_ITRANSACTIONALDICTIONARY_H

#include <stingray/toolkit/collection/DiffEntry.h>
#include <stingray/toolkit/collection/IDictionary.h>
#include <stingray/toolkit/collection/ObservableCollectionLocker.h>
#include <stingray/toolkit/signal/signals.h>
#include <stingray/toolkit/toolkit.h>

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

