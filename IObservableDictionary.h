#ifndef STINGRAY_TOOLKIT_IOBSERVABLEDICTIONARY_H
#define STINGRAY_TOOLKIT_IOBSERVABLEDICTIONARY_H


#include <stingray/toolkit/IDictionary.h>
#include <stingray/toolkit/signals.h>
#include <stingray/toolkit/toolkit.h>


namespace stingray
{

	class ObservableCollectionLocker
	{
		TOOLKIT_NONCOPYABLE(ObservableCollectionLocker);

	private:
		signal_locker _locker;

	public:
		template < typename CollectionType >
		ObservableCollectionLocker(const CollectionType& collection)
			: _locker(collection.OnChanged)
		{ }
	};
	TOOLKIT_DECLARE_PTR(ObservableCollectionLocker);


	template < typename KeyType_, typename ValueType_ >
	struct IObservableDictionary : public virtual IDictionary<KeyType_, ValueType_>
	{
		signal<void(CollectionOp, KeyType_, ValueType_)>	OnChanged;

		ObservableCollectionLockerPtr Lock() const { return make_shared<ObservableCollectionLocker>(*this); }

	protected:
		IObservableDictionary()
			//: OnChanged(bind(&IObservableDictionary::OnChangedPopulator, this, _1))
		{ }

		virtual void OnChangedPopulator(const function<void(CollectionOp, KeyType_, ValueType_)>& slot)
		{
			typedef KeyValuePair<KeyType_, ValueType_>	PairType;
			FOR_EACH(PairType p IN this->GetEnumerator())
				slot(CollectionOp::ItemAdded, p.Key, p.Value);
		}
	};


	template < typename Wrapped_ >
	struct ObservableDictionaryWrapper
		:	public Wrapped_,
			public virtual IObservableDictionary<typename Wrapped_::KeyType, typename Wrapped_::ValueType>
	{
		typedef typename Wrapped_::KeyType						KeyType;
		typedef typename Wrapped_::ValueType					ValueType;
		typedef IObservableDictionary<KeyType, ValueType>		ObservableInterface;

		virtual int GetCount() const
		{
			signal_locker l(ObservableInterface::OnChanged);
			return Wrapped_::GetCount();
		}

		virtual ValueType Get(const KeyType& key) const
		{
			signal_locker l(ObservableInterface::OnChanged);
			return Wrapped_::Get(key);
		}

		virtual void Set(const KeyType& key, const ValueType& value)
		{
			signal_locker l(ObservableInterface::OnChanged);
			bool update = ContainsKey(key);
			Wrapped_::Set(key, value);
			ObservableInterface::OnChanged(update ? CollectionOp::ItemUpdated : CollectionOp::ItemAdded, key, value);
		}

		virtual void Remove(const KeyType& key)
		{
			signal_locker l(ObservableInterface::OnChanged);
			ValueType value = Get(key);
			Wrapped_::Remove(key);
			ObservableInterface::OnChanged(CollectionOp::ItemRemoved, key, value);
		}

		virtual bool ContainsKey(const KeyType& key) const
		{
			signal_locker l(ObservableInterface::OnChanged);
			return Wrapped_::ContainsKey(key);
		}

		virtual bool TryGet(const KeyType& key, ValueType& outValue) const
		{
			signal_locker l(ObservableInterface::OnChanged);
			return Wrapped_::TryGet(key, outValue);
		}

		virtual void Clear()
		{
			signal_locker l(ObservableInterface::OnChanged);
			Wrapped_::Clear();
		}
	};

}


#endif
