#ifndef STINGRAY_TOOLKIT_IOBSERVABLEDICTIONARY_H
#define STINGRAY_TOOLKIT_IOBSERVABLEDICTIONARY_H


#include <stingray/toolkit/IDictionary.h>
#include <stingray/toolkit/ObservableCollectionLocker.h>
#include <stingray/toolkit/signals.h>
#include <stingray/toolkit/toolkit.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename KeyType_, typename ValueType_ >
	struct IObservableDictionary : public virtual IDictionary<KeyType_, ValueType_>
	{
		virtual signal_connector<void(CollectionOp, KeyType_, ValueType_)>	OnChanged() const = 0;

		ObservableCollectionLockerPtr Lock() const { return make_shared<ObservableCollectionLocker>(*this); }

	protected:
		IObservableDictionary() { }

	public:
		virtual const Mutex& GetSyncRoot() const = 0;
	};


	template < typename Wrapped_ >
	struct ObservableDictionaryWrapper
		:	public Wrapped_,
			public virtual IObservableDictionary<typename Wrapped_::KeyType, typename Wrapped_::ValueType>
	{
	public:
		typedef typename Wrapped_::KeyType						KeyType;
		typedef typename Wrapped_::ValueType					ValueType;
		typedef typename Wrapped_::PairType						PairType;
		typedef IObservableDictionary<KeyType, ValueType>		ObservableInterface;

	private:
		Mutex																						_mutex;
		signal<void(CollectionOp, KeyType, ValueType), signal_policies::threading::ExternalMutex>	_onChanged;

	public:
		ObservableDictionaryWrapper()
			: _onChanged(signal_policies::threading::ExternalMutex(_mutex), bind(&ObservableDictionaryWrapper::OnChangedPopulator, this, _1))
		{ }

		virtual const Mutex& GetSyncRoot() const { return _mutex; }

		virtual signal_connector<void(CollectionOp, KeyType, ValueType)>	OnChanged() const
		{ return _onChanged.connector(); }

		virtual int GetCount() const
		{
			signal_locker l(_onChanged);
			return Wrapped_::GetCount();
		}

		virtual bool IsEmpty() const
		{
			signal_locker l(_onChanged);
			return Wrapped_::IsEmpty();
		}

		virtual ValueType Get(const KeyType& key) const
		{
			signal_locker l(_onChanged);
			return Wrapped_::Get(key);
		}

		virtual void Set(const KeyType& key, const ValueType& value)
		{
			signal_locker l(_onChanged);
			bool update = ContainsKey(key);
			Wrapped_::Set(key, value);
			_onChanged(update ? CollectionOp::Updated : CollectionOp::Added, key, value);
		}

		virtual void Remove(const KeyType& key)
		{
			signal_locker l(_onChanged);
			ValueType value = Get(key);
			Wrapped_::Remove(key);
			_onChanged(CollectionOp::Removed, key, value);
		}

		virtual bool ContainsKey(const KeyType& key) const
		{
			signal_locker l(_onChanged);
			return Wrapped_::ContainsKey(key);
		}

		virtual bool TryGet(const KeyType& key, ValueType& outValue) const
		{
			signal_locker l(_onChanged);
			return Wrapped_::TryGet(key, outValue);
		}

		virtual bool TryRemove(const KeyType& key)
		{
			signal_locker l(_onChanged);
			ValueType value;
			if (!TryGet(key, value))
				return false;

			_onChanged(CollectionOp::Removed, key, value);
			Wrapped_::Remove(key);
			return true;
		}

		virtual void Clear()
		{
			signal_locker l(_onChanged);
			FOR_EACH(PairType v IN this->GetEnumerator())
				_onChanged(CollectionOp::Removed, v.Key, v.Value);
			Wrapped_::Clear();
		}

		virtual shared_ptr<IEnumerator<PairType> > GetEnumerator() const
		{
			signal_locker l(_onChanged);
			return Wrapped_::GetEnumerator();
		}

		virtual shared_ptr<IEnumerable<PairType> > Reverse() const
		{
			signal_locker l(_onChanged);
			return Wrapped_::Reverse();
		}

	private:
		void OnChangedPopulator(const function<void(CollectionOp, KeyType, ValueType)>& slot) const
		{
			FOR_EACH(PairType p IN this->GetEnumerator())
				slot(CollectionOp::Added, p.Key, p.Value);
		}
	};

	/** @} */

}


#endif
