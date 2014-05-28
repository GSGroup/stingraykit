#ifndef STINGRAY_TOOLKIT_IOBSERVABLESET_H
#define STINGRAY_TOOLKIT_IOBSERVABLESET_H


#include <stingray/toolkit/ISet.h>
#include <stingray/toolkit/signals.h>
#include <stingray/toolkit/toolkit.h>
#include <stingray/toolkit/ObservableCollectionLocker.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename ValueType_ >
	struct IObservableSet : public virtual ISet<ValueType_>
	{
		virtual signal_connector<void(CollectionOp, const ValueType_&)>	OnChanged() const = 0;
		virtual const Mutex& GetSyncRoot() const = 0;

	protected:
		virtual void InvokeOnChanged(CollectionOp, const ValueType_&) = 0;
	};


	template < typename Wrapped_ >
	struct ObservableSetWrapper
		:	public  Wrapped_,
			public virtual IObservableSet<typename Wrapped_::ValueType>
	{
		typedef typename Wrapped_::ValueType	ValueType;
		typedef IObservableSet<ValueType>		ObservableInterface;

		virtual int GetCount() const
		{
			signal_locker l(ObservableInterface::OnChanged);
			return Wrapped_::GetCount();
		}

		virtual void Add(const ValueType& value)
		{
			signal_locker l(ObservableInterface::OnChanged);
			bool signal = !Wrapped_::Contains(value);
			Wrapped_::Add(value);
			if (signal)
				ObservableInterface::OnChanged(CollectionOp::Added, value);
		}

		virtual void Remove(const ValueType& value)
		{
			signal_locker l(ObservableInterface::OnChanged);
			bool signal = Wrapped_::Contains(value);
			Wrapped_::Remove(value);
			if (signal)
				ObservableInterface::OnChanged(CollectionOp::Removed, value);
		}

		virtual bool TryRemove(const ValueType& value)
		{
			signal_locker l(ObservableInterface::OnChanged);
			if (!Wrapped_::TryRemove(value))
				return false;

			ObservableInterface::OnChanged(CollectionOp::Removed, value);
			return true;
		}

		virtual bool Contains(const ValueType& value) const
		{
			signal_locker l(ObservableInterface::OnChanged);
			return Wrapped_::Contains(value);
		}

		virtual void Clear()
		{
			signal_locker l(ObservableInterface::OnChanged);
			FOR_EACH(ValueType v IN this->GetEnumerator())
				ObservableInterface::OnChanged(CollectionOp::Removed, v);
			Wrapped_::Clear();
		}

		virtual shared_ptr<IEnumerator<ValueType> > GetEnumerator() const
		{
			signal_locker l(ObservableInterface::OnChanged);
			return Wrapped_::GetEnumerator();
		}

		virtual shared_ptr<IEnumerable<ValueType> > Reverse() const
		{
			signal_locker l(ObservableInterface::OnChanged);
			return Wrapped_::Reverse();
		}
	};

	/** @} */

}


#endif
