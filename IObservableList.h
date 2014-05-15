#ifndef STINGRAY_TOOLKIT_IOBSERVABLELIST_H
#define STINGRAY_TOOLKIT_IOBSERVABLELIST_H


#include <stingray/toolkit/IList.h>
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
	struct IObservableList : public virtual IList<ValueType_>
	{
		signal<void(CollectionOp, int, const ValueType_&)>	OnChanged;

		ObservableCollectionLockerPtr Lock() const { return make_shared<ObservableCollectionLocker>(*this); }

	protected:
		IObservableList()
			: OnChanged(bind(&IObservableList::OnChangedPopulator, this, _1))
		{ }

		virtual void OnChangedPopulator(const function<void(CollectionOp, int, const ValueType_&)>& slot)
		{
			int i = 0;
			FOR_EACH(ValueType_ v IN this->GetEnumerator())
				slot(CollectionOp::Added, i++, v);
		}
	};


	template < typename Wrapped_ >
	struct ObservableListWrapper
		:	public Wrapped_,
			public virtual IObservableList<typename Wrapped_::ValueType>
	{
		typedef typename Wrapped_::ValueType	ValueType;
		typedef IObservableList<ValueType>		ObservableInterface;

		virtual int GetCount() const
		{
			signal_locker l(ObservableInterface::OnChanged);
			return Wrapped_::GetCount();
		}

		virtual void Add(const ValueType& value)
		{
			signal_locker l(ObservableInterface::OnChanged);
			Wrapped_::Add(value);
			ObservableInterface::OnChanged(CollectionOp::Added, Wrapped_::GetCount() - 1, value);
		}

		virtual ValueType Get(int index) const
		{
			signal_locker l(ObservableInterface::OnChanged);
			return Wrapped_::Get(index);
		}

		virtual void Set(int index, const ValueType& value)
		{
			signal_locker l(ObservableInterface::OnChanged);
			Wrapped_::Set(index, value);
			ObservableInterface::OnChanged(CollectionOp::Updated, index, value);
		}

		virtual int IndexOf(const ValueType& obj) const
		{
			signal_locker l(ObservableInterface::OnChanged);
			return Wrapped_::IndexOf(obj);
		}

		virtual void Insert(int index, const ValueType& value)
		{
			signal_locker l(ObservableInterface::OnChanged);
			Wrapped_::Insert(index, value);
			ObservableInterface::OnChanged(CollectionOp::Added, index, value);
		}

		virtual void RemoveAt(int index)
		{
			signal_locker l(ObservableInterface::OnChanged);
			ValueType value = Get(index);
			Wrapped_::RemoveAt(index);
			ObservableInterface::OnChanged(CollectionOp::Removed, index, value);
		}

		virtual void Remove(const ValueType& value)
		{
			signal_locker l(ObservableInterface::OnChanged);
			int index = Wrapped_::IndexOf(value);
			if (index != -1)
				RemoveAt(index);
		}

		virtual bool Contains(const ValueType& value) const
		{
			signal_locker l(ObservableInterface::OnChanged);
			return Wrapped_::Contains(value);
		}

		virtual bool TryGet(int index, ValueType& value) const
		{
			signal_locker l(ObservableInterface::OnChanged);
			return Wrapped_::TryGet(index, value);
		}

		virtual void Clear()
		{
			signal_locker l(ObservableInterface::OnChanged);
			while (!this->IsEmpty())
				RemoveAt(0);
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
