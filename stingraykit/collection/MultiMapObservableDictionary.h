#ifndef STINGRAYKIT_COLLECTION_MULTIMAPOBSERVABLEDICTIONARY_H
#define STINGRAYKIT_COLLECTION_MULTIMAPOBSERVABLEDICTIONARY_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/IObservableMultiDictionary.h>
#include <stingraykit/collection/MultiMapDictionary.h>
#include <stingraykit/signal/signals.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename KeyType_, typename ValueType_ , typename KeyCompareType_ = comparers::Less, typename ValueCompareType_ = comparers::Equals >
	struct MultiMapObservableDictionary
		:	public MultiMapDictionary<KeyType_, ValueType_, KeyCompareType_, ValueCompareType_>,
			public virtual IObservableMultiDictionary<KeyType_, ValueType_>
	{
		typedef MultiMapDictionary<KeyType_, ValueType_, KeyCompareType_, ValueCompareType_> Wrapped;

		typedef signal_policies::threading::ExternalMutexPointer ExternalMutexPointer;

	public:
		typedef typename Wrapped::KeyType							KeyType;
		typedef typename Wrapped::ValueType							ValueType;
		typedef typename Wrapped::PairType							PairType;
		typedef IObservableMultiDictionary<KeyType, ValueType>		ObservableInterface;
		typedef typename ObservableInterface::OnChangedSignature	OnChangedSignature;

	private:
		shared_ptr<Mutex>									_mutex;
		signal<OnChangedSignature, ExternalMutexPointer>	_onChanged;

	public:
		MultiMapObservableDictionary()
			:	Wrapped(),
				_mutex(make_shared<Mutex>()),
				_onChanged(ExternalMutexPointer(_mutex), bind(&MultiMapObservableDictionary::OnChangedPopulator, this, _1))
		{ }

		MultiMapObservableDictionary(shared_ptr<IEnumerable<PairType> > enumerable)
			:	Wrapped(enumerable),
				_mutex(make_shared<Mutex>()),
				_onChanged(ExternalMutexPointer(_mutex), bind(&MultiMapObservableDictionary::OnChangedPopulator, this, _1))
		{ }

		MultiMapObservableDictionary(shared_ptr<IEnumerator<PairType> > enumerator)
			:	Wrapped(enumerator),
				_mutex(make_shared<Mutex>()),
				_onChanged(ExternalMutexPointer(_mutex), bind(&MultiMapObservableDictionary::OnChangedPopulator, this, _1))
		{ }

		virtual shared_ptr<IEnumerator<PairType> > GetEnumerator() const
		{
			signal_locker l(_onChanged);
			return Wrapped::GetEnumerator();
		}

		virtual shared_ptr<IEnumerable<PairType> > Reverse() const
		{
			signal_locker l(_onChanged);
			return Wrapped::Reverse();
		}

		virtual size_t GetCount() const
		{
			signal_locker l(_onChanged);
			return Wrapped::GetCount();
		}

		virtual bool IsEmpty() const
		{
			signal_locker l(_onChanged);
			return Wrapped::IsEmpty();
		}

		virtual bool ContainsKey(const KeyType& key) const
		{
			signal_locker l(_onChanged);
			return Wrapped::ContainsKey(key);
		}

		virtual size_t CountKey(const KeyType& key) const
		{
			signal_locker l(_onChanged);
			return Wrapped::CountKey(key);
		}

		virtual shared_ptr<IEnumerator<PairType> > Find(const KeyType& key) const
		{
			signal_locker l(_onChanged);
			return Wrapped::Find(key);
		}

		virtual shared_ptr<IEnumerator<PairType> > ReverseFind(const KeyType& key) const
		{
			signal_locker l(_onChanged);
			return Wrapped::ReverseFind(key);
		}

		virtual ValueType GetFirst(const KeyType& key) const
		{
			signal_locker l(_onChanged);
			return Wrapped::GetFirst(key);
		}

		virtual bool TryGetFirst(const KeyType& key, ValueType& outValue) const
		{
			signal_locker l(_onChanged);
			return Wrapped::TryGetFirst(key, outValue);
		}

		virtual shared_ptr<IEnumerator<PairType> > GetAll(const KeyType& key) const
		{
			signal_locker l(_onChanged);
			return Wrapped::GetAll(key);
		}

		virtual void Set(const KeyType& key, const ValueType& value)
		{
			signal_locker l(_onChanged);
			Wrapped::Set(key, value);
			_onChanged(CollectionOp::Added, key, value);
		}

		virtual void RemoveFirst(const KeyType& key, const optional<ValueType>& value = null)
		{ MultiMapObservableDictionary::TryRemoveFirst(key, value); }

		virtual bool TryRemoveFirst(const KeyType& key, const optional<ValueType>& value = null)
		{
			signal_locker l(_onChanged);
			FOR_EACH(PairType v IN Wrapped::GetAll(key))
			{
				if (value && !ValueCompareType_()(*value, v.Value))
					continue;

				Wrapped::RemoveFirst(v.Key, v.Value);
				_onChanged(CollectionOp::Removed, v.Key, v.Value);
				return true;
			}
			return false;
		}

		virtual size_t RemoveAll(const KeyType& key)
		{
			signal_locker l(_onChanged);
			size_t ret = 0;
			FOR_EACH(PairType v IN Wrapped::Find(key))
			{
				if (KeyCompareType_()(v.Key, key) || KeyCompareType_()(key, v.Key))
					break;

				Wrapped::RemoveFirst(v.Key, v.Value);
				_onChanged(CollectionOp::Removed, v.Key, v.Value);
				++ret;
			}
			return ret;
		}

		virtual size_t RemoveWhere(const function<bool (const KeyType&, const ValueType&)>& pred)
		{
			signal_locker l(_onChanged);
			size_t ret = 0;
			FOR_EACH(PairType v IN Wrapped::GetEnumerator() WHERE pred(v.Key, v.Value))
			{
				Wrapped::RemoveFirst(v.Key, v.Value);
				_onChanged(CollectionOp::Removed, v.Key, v.Value);
				++ret;
			}
			return ret;
		}

		virtual void Clear()
		{
			signal_locker l(_onChanged);
			FOR_EACH(PairType v IN Wrapped::GetEnumerator())
			{
				Wrapped::RemoveFirst(v.Key, v.Value);
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
			FOR_EACH(PairType p IN Wrapped::GetEnumerator())
				slot(CollectionOp::Added, p.Key, p.Value);
		}
	};

	/** @} */

}

#endif
