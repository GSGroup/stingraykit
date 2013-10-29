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
				slot(CollectionOp::Added, p.Key, p.Value);
		}
	};


	namespace Detail
	{
		template < typename T, typename Base, bool IsSerializable_ = Inherits<T, ISerializable>::Value >
		struct ObservableSerializableDictionary : public Base
		{
			virtual void Serialize(ObjectOStream & ar) const
			{
				ObservableCollectionLockerPtr l(static_cast<const T*>(this)->Lock());
				Base::Serialize(ar);
			}

			virtual void Deserialize(ObjectIStream & ar)
			{
				ObservableCollectionLockerPtr l(static_cast<const T*>(this)->Lock());
				Base::Deserialize(ar);
			}
		};

		template < typename T, typename Base >
		struct ObservableSerializableDictionary<T, Base, false> : public Base
		{ };
	}


	template < typename Wrapped_ >
	struct ObservableDictionaryWrapper
		:	public Detail::ObservableSerializableDictionary<ObservableDictionaryWrapper<Wrapped_>, Wrapped_>,
			public virtual IObservableDictionary<typename Wrapped_::KeyType, typename Wrapped_::ValueType>
	{
		typedef typename Wrapped_::KeyType						KeyType;
		typedef typename Wrapped_::ValueType					ValueType;
		typedef typename Wrapped_::PairType						PairType;
		typedef IObservableDictionary<KeyType, ValueType>		ObservableInterface;

		virtual int GetCount() const
		{
			signal_locker l(ObservableInterface::OnChanged);
			return Wrapped_::GetCount();
		}

		virtual bool IsEmpty() const
		{
			signal_locker l(ObservableInterface::OnChanged);
			return Wrapped_::IsEmpty();
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
			ObservableInterface::OnChanged(update ? CollectionOp::Updated : CollectionOp::Added, key, value);
		}

		virtual void Remove(const KeyType& key)
		{
			signal_locker l(ObservableInterface::OnChanged);
			ValueType value = Get(key);
			Wrapped_::Remove(key);
			ObservableInterface::OnChanged(CollectionOp::Removed, key, value);
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

		virtual bool TryRemove(const KeyType& key)
		{
			signal_locker l(ObservableInterface::OnChanged);
			return Wrapped_::TryRemove(key);
		}

		virtual void Clear()
		{
			signal_locker l(ObservableInterface::OnChanged);
			Wrapped_::Clear();
		}

		virtual shared_ptr<IEnumerator<PairType> > GetEnumerator() const
		{
			signal_locker l(ObservableInterface::OnChanged);
			return Wrapped_::GetEnumerator();
		}

		virtual shared_ptr<IEnumerable<PairType> > Reverse() const
		{
			signal_locker l(ObservableInterface::OnChanged);
			return Wrapped_::Reverse();
		}
	};

	/** @} */

}


#endif
