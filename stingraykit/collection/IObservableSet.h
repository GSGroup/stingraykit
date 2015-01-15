#ifndef STINGRAYKIT_COLLECTION_IOBSERVABLESET_H
#define STINGRAYKIT_COLLECTION_IOBSERVABLESET_H


#include <stingraykit/collection/ISet.h>
#include <stingraykit/signal/signals.h>
#include <stingraykit/toolkit.h>
#include <stingraykit/collection/ObservableCollectionLocker.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename ValueType_ >
	struct IReadonlyObservableSet : public virtual IReadonlySet<ValueType_>
	{
		virtual signal_connector<void(CollectionOp, const ValueType_&)>	OnChanged() const = 0;
		virtual const Mutex& GetSyncRoot() const = 0;

	protected:
		virtual void InvokeOnChanged(CollectionOp, const ValueType_&) = 0;
	};

	template < typename ValueType_ >
	struct IObservableSet : public virtual ISet<ValueType_>, public virtual IReadonlyObservableSet<ValueType_>
	{
	};


	template < typename Wrapped_ >
	struct ObservableSetWrapper
		:	public Wrapped_,
			public virtual IObservableSet<typename Wrapped_::ValueType>
	{
		typedef typename Wrapped_::ValueType					ValueType;

		virtual int GetCount() const
		{
			MutexLock l(this->GetSyncRoot());
			return Wrapped_::GetCount();
		}

		virtual void Add(const ValueType& value)
		{
			MutexLock l(this->GetSyncRoot());
			bool signal = !Wrapped_::Contains(value);
			Wrapped_::Add(value);
			if (signal)
				this->InvokeOnChanged(CollectionOp::Added, value);
		}

		virtual void Remove(const ValueType& value)
		{
			MutexLock l(this->GetSyncRoot());
			bool signal = Wrapped_::Contains(value);
			Wrapped_::Remove(value);
			if (signal)
				this->InvokeOnChanged(CollectionOp::Removed, value);
		}

		virtual bool TryRemove(const ValueType& value)
		{
			MutexLock l(this->GetSyncRoot());
			if (!Wrapped_::TryRemove(value))
				return false;

			this->InvokeOnChanged(CollectionOp::Removed, value);
			return true;
		}

		virtual bool Contains(const ValueType& value) const
		{
			MutexLock l(this->GetSyncRoot());
			return Wrapped_::Contains(value);
		}

		virtual void Clear()
		{
			MutexLock l(this->GetSyncRoot());
			FOR_EACH(ValueType v IN this->GetEnumerator())
				this->InvokeOnChanged(CollectionOp::Removed, v);
			Wrapped_::Clear();
		}

		virtual shared_ptr<IEnumerator<ValueType> > GetEnumerator() const
		{
			MutexLock l(this->GetSyncRoot());
			return Wrapped_::GetEnumerator();
		}

		virtual shared_ptr<IEnumerable<ValueType> > Reverse() const
		{
			MutexLock l(this->GetSyncRoot());
			return Wrapped_::Reverse();
		}
	};

	/** @} */

}


#endif
