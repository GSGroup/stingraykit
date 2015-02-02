#ifndef STINGRAYKIT_COLLECTION_IOBSERVABLELIST_H
#define STINGRAYKIT_COLLECTION_IOBSERVABLELIST_H


#include <stingraykit/collection/ForEach.h>
#include <stingraykit/collection/IList.h>
#include <stingraykit/collection/ObservableCollectionLocker.h>
#include <stingraykit/signal/signals.h>
#include <stingraykit/toolkit.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename ValueType_ >
	struct IObservableList : public virtual IList<ValueType_>
	{
		virtual signal_connector<void(CollectionOp, int, const ValueType_&)> OnChanged() const = 0;

		ObservableCollectionLockerPtr Lock() const { return make_shared<ObservableCollectionLocker>(*this); }

		virtual const Mutex& GetSyncRoot() const = 0;
	};


	template < typename Wrapped_ >
	struct ObservableListWrapper
		:	public Wrapped_,
			public virtual IObservableList<typename Wrapped_::ValueType>
	{
	public:
		typedef typename Wrapped_::ValueType	ValueType;
		typedef IObservableList<ValueType>		ObservableInterface;

	private:
		shared_ptr<Mutex>																					_mutex;
		signal<void(CollectionOp, int, const ValueType&), signal_policies::threading::ExternalMutexPointer>	_onChanged;

	public:
		ObservableListWrapper()
			: _mutex(new Mutex()), _onChanged(signal_policies::threading::ExternalMutexPointer(_mutex), bind(&ObservableListWrapper::OnChangedPopulator, this, _1))
		{ }

		virtual signal_connector<void(CollectionOp, int, const ValueType&)> OnChanged() const
		{ return _onChanged.connector(); }

		virtual const Mutex& GetSyncRoot() const { return *_mutex; }

		virtual int GetCount() const
		{
			signal_locker l(_onChanged);
			return Wrapped_::GetCount();
		}

		virtual void Add(const ValueType& value)
		{
			signal_locker l(_onChanged);
			Wrapped_::Add(value);
			_onChanged(CollectionOp::Added, Wrapped_::GetCount() - 1, value);
		}

		virtual ValueType Get(int index) const
		{
			signal_locker l(_onChanged);
			return Wrapped_::Get(index);
		}

		virtual void Set(int index, const ValueType& value)
		{
			signal_locker l(_onChanged);
			Wrapped_::Set(index, value);
			_onChanged(CollectionOp::Updated, index, value);
		}

		virtual int IndexOf(const ValueType& obj) const
		{
			signal_locker l(_onChanged);
			return Wrapped_::IndexOf(obj);
		}

		virtual void Insert(int index, const ValueType& value)
		{
			signal_locker l(_onChanged);
			Wrapped_::Insert(index, value);
			_onChanged(CollectionOp::Added, index, value);
		}

		virtual void RemoveAt(int index)
		{
			signal_locker l(_onChanged);
			ValueType value = Get(index);
			Wrapped_::RemoveAt(index);
			_onChanged(CollectionOp::Removed, index, value);
		}

		virtual void Remove(const ValueType& value)
		{
			signal_locker l(_onChanged);
			int index = Wrapped_::IndexOf(value);
			if (index != -1)
				RemoveAt(index);
		}

		virtual bool Contains(const ValueType& value) const
		{
			signal_locker l(_onChanged);
			return Wrapped_::Contains(value);
		}

		virtual bool TryGet(int index, ValueType& value) const
		{
			signal_locker l(_onChanged);
			return Wrapped_::TryGet(index, value);
		}

		virtual void Clear()
		{
			signal_locker l(_onChanged);
			while (!this->IsEmpty())
				RemoveAt(0);
		}

		virtual shared_ptr<IEnumerator<ValueType> > GetEnumerator() const
		{
			signal_locker l(_onChanged);
			return Wrapped_::GetEnumerator();
		}

		virtual shared_ptr<IEnumerable<ValueType> > Reverse() const
		{
			signal_locker l(_onChanged);
			return Wrapped_::Reverse();
		}

	private:
		virtual void OnChangedPopulator(const function<void(CollectionOp, int, const ValueType&)>& slot)
		{
			int i = 0;
			FOR_EACH(ValueType v IN this->GetEnumerator())
				slot(CollectionOp::Added, i++, v);
		}
	};

	/** @} */

}


#endif
