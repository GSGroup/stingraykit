#ifndef STINGRAYKIT_COLLECTION_OBSERVABLEDICTIONARYWRAPPER_H
#define STINGRAYKIT_COLLECTION_OBSERVABLEDICTIONARYWRAPPER_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/ForEach.h>
#include <stingraykit/collection/IObservableDictionary.h>
#include <stingraykit/signal/signals.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename Wrapped_ >
	struct ObservableDictionaryWrapper
		:	public Wrapped_,
			public virtual IObservableDictionary<typename Wrapped_::KeyType, typename Wrapped_::ValueType>
	{
		typedef signal_policies::threading::ExternalMutexPointer ExternalMutexPointer;

	public:
		typedef typename Wrapped_::KeyType							KeyType;
		typedef typename Wrapped_::ValueType						ValueType;
		typedef typename Wrapped_::PairType							PairType;
		typedef IObservableDictionary<KeyType, ValueType>			ObservableInterface;
		typedef typename ObservableInterface::OnChangedSignature	OnChangedSignature;

	private:
		shared_ptr<Mutex>									_mutex;
		signal<OnChangedSignature, ExternalMutexPointer>	_onChanged;

	public:
		ObservableDictionaryWrapper()
			:	Wrapped_(),
				_mutex(make_shared<Mutex>()),
				_onChanged(ExternalMutexPointer(_mutex), bind(&ObservableDictionaryWrapper::OnChangedPopulator, this, _1))
		{ }

		ObservableDictionaryWrapper(shared_ptr<IEnumerable<PairType> > enumerable)
			:	Wrapped_(enumerable),
				_mutex(make_shared<Mutex>()),
				_onChanged(ExternalMutexPointer(_mutex), bind(&ObservableDictionaryWrapper::OnChangedPopulator, this, _1))
		{ }

		ObservableDictionaryWrapper(shared_ptr<IEnumerator<PairType> > enumerator)
			:	Wrapped_(enumerator),
				_mutex(make_shared<Mutex>()),
				_onChanged(ExternalMutexPointer(_mutex), bind(&ObservableDictionaryWrapper::OnChangedPopulator, this, _1))
		{ }

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

		virtual size_t GetCount() const
		{
			signal_locker l(_onChanged);
			return Wrapped_::GetCount();
		}

		virtual bool IsEmpty() const
		{
			signal_locker l(_onChanged);
			return Wrapped_::IsEmpty();
		}

		virtual bool ContainsKey(const KeyType& key) const
		{
			signal_locker l(_onChanged);
			return Wrapped_::ContainsKey(key);
		}

		virtual ValueType Get(const KeyType& key) const
		{
			signal_locker l(_onChanged);
			return Wrapped_::Get(key);
		}

		virtual bool TryGet(const KeyType& key, ValueType& outValue) const
		{
			signal_locker l(_onChanged);
			return Wrapped_::TryGet(key, outValue);
		}

		virtual void Set(const KeyType& key, const ValueType& value)
		{
			signal_locker l(_onChanged);
			bool update = ContainsKey(key);
			Wrapped_::Set(key, value);
			_onChanged(update ? CollectionOp::Updated : CollectionOp::Added, key, value);
		}

		virtual void Remove(const KeyType& key)
		{ TryRemove(key); }

		virtual bool TryRemove(const KeyType& key)
		{
			signal_locker l(_onChanged);
			ValueType value;
			if (!TryGet(key, value))
				return false;

			Wrapped_::Remove(key);
			_onChanged(CollectionOp::Removed, key, value);
			return true;
		}

		virtual size_t RemoveWhere(const function<bool (const KeyType&, const ValueType&)>& pred)
		{
			signal_locker l(_onChanged);
			size_t ret = 0;
			FOR_EACH(PairType v IN GetEnumerator() WHERE pred(v.Key, v.Value))
			{
				Wrapped_::Remove(v.Key);
				_onChanged(CollectionOp::Removed, v.Key, v.Value);
				++ret;
			}
			return ret;
		}

		virtual void Clear()
		{
			signal_locker l(_onChanged);
			FOR_EACH(PairType v IN GetEnumerator())
			{
				Wrapped_::Remove(v.Key);
				_onChanged(CollectionOp::Removed, v.Key, v.Value);
			}
		}

		virtual signal_connector<OnChangedSignature> OnChanged() const
		{ return _onChanged.connector(); }

		virtual const Mutex& GetSyncRoot() const
		{ return *_mutex; }

	private:
		void OnChangedPopulator(const function<OnChangedSignature>& slot) const
		{
			FOR_EACH(PairType p IN this->GetEnumerator())
				slot(CollectionOp::Added, p.Key, p.Value);
		}
	};

	/** @} */

}

#endif
