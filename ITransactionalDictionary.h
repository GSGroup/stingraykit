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
				TOOLKIT_THROW("Not supported CollectionOp");
			}
		}

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


}


#endif

