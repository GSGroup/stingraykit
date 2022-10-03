#ifndef STINGRAYKIT_COLLECTION_OBSERVABLEDICTIONARYWRAPPER_H
#define STINGRAYKIT_COLLECTION_OBSERVABLEDICTIONARYWRAPPER_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
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
	class ObservableDictionaryWrapper : public virtual IObservableDictionary<typename Wrapped_::KeyType, typename Wrapped_::ValueType>
	{
		using ExternalMutexPointer = signal_policies::threading::ExternalMutexPointer;

	public:
		using KeyType = typename Wrapped_::KeyType;
		using ValueType = typename Wrapped_::ValueType;
		using PairType = typename Wrapped_::PairType;
		using ObservableInterface = IObservableDictionary<KeyType, ValueType>;
		using OnChangedSignature = typename ObservableInterface::OnChangedSignature;

	private:
		Wrapped_											_wrapped;
		shared_ptr<Mutex>									_mutex;
		signal<OnChangedSignature, ExternalMutexPointer>	_onChanged;

	public:
		ObservableDictionaryWrapper()
			:	_mutex(make_shared_ptr<Mutex>()),
				_onChanged(ExternalMutexPointer(_mutex), Bind(&ObservableDictionaryWrapper::OnChangedPopulator, this, _1))
		{ }

		explicit ObservableDictionaryWrapper(const shared_ptr<IEnumerable<PairType>>& enumerable)
			:	_wrapped(enumerable),
				_mutex(make_shared_ptr<Mutex>()),
				_onChanged(ExternalMutexPointer(_mutex), Bind(&ObservableDictionaryWrapper::OnChangedPopulator, this, _1))
		{ }

		explicit ObservableDictionaryWrapper(const shared_ptr<IEnumerator<PairType>>& enumerator)
			:	_wrapped(enumerator),
				_mutex(make_shared_ptr<Mutex>()),
				_onChanged(ExternalMutexPointer(_mutex), Bind(&ObservableDictionaryWrapper::OnChangedPopulator, this, _1))
		{ }

		shared_ptr<IEnumerator<PairType>> GetEnumerator() const override
		{
			signal_locker l(_onChanged);
			return _wrapped.GetEnumerator();
		}

		shared_ptr<IEnumerable<PairType>> Reverse() const override
		{
			signal_locker l(_onChanged);
			return _wrapped.Reverse();
		}

		size_t GetCount() const override
		{
			signal_locker l(_onChanged);
			return _wrapped.GetCount();
		}

		bool IsEmpty() const override
		{
			signal_locker l(_onChanged);
			return _wrapped.IsEmpty();
		}

		bool ContainsKey(const KeyType& key) const override
		{
			signal_locker l(_onChanged);
			return _wrapped.ContainsKey(key);
		}

		shared_ptr<IEnumerator<PairType>> Find(const KeyType& key) const override
		{
			signal_locker l(_onChanged);
			return _wrapped.Find(key);
		}

		shared_ptr<IEnumerator<PairType>> ReverseFind(const KeyType& key) const override
		{
			signal_locker l(_onChanged);
			return _wrapped.ReverseFind(key);
		}

		ValueType Get(const KeyType& key) const override
		{
			signal_locker l(_onChanged);
			return _wrapped.Get(key);
		}

		bool TryGet(const KeyType& key, ValueType& outValue) const override
		{
			signal_locker l(_onChanged);
			return _wrapped.TryGet(key, outValue);
		}

		void Set(const KeyType& key, const ValueType& value) override
		{
			signal_locker l(_onChanged);
			bool update = _wrapped.ContainsKey(key);
			_wrapped.Set(key, value);
			_onChanged(update ? CollectionOp::Updated : CollectionOp::Added, key, value);
		}

		void Remove(const KeyType& key) override
		{ TryRemove(key); }

		bool TryRemove(const KeyType& key) override
		{
			signal_locker l(_onChanged);
			ValueType value;
			if (!_wrapped.TryGet(key, value))
				return false;

			_wrapped.Remove(key);
			_onChanged(CollectionOp::Removed, key, value);
			return true;
		}

		size_t RemoveWhere(const function<bool (const KeyType&, const ValueType&)>& pred) override
		{
			signal_locker l(_onChanged);
			size_t ret = 0;
			FOR_EACH(PairType v IN _wrapped.GetEnumerator() WHERE pred(v.Key, v.Value))
			{
				_wrapped.Remove(v.Key);
				_onChanged(CollectionOp::Removed, v.Key, v.Value);
				++ret;
			}
			return ret;
		}

		void Clear() override
		{
			signal_locker l(_onChanged);
			FOR_EACH(PairType v IN _wrapped.GetEnumerator())
			{
				_wrapped.Remove(v.Key);
				_onChanged(CollectionOp::Removed, v.Key, v.Value);
			}
		}

		signal_connector<OnChangedSignature> OnChanged() const override
		{ return _onChanged.connector(); }

		const Mutex& GetSyncRoot() const override
		{ return *_mutex; }

	private:
		void OnChangedPopulator(const function<OnChangedSignature>& slot) const
		{
			FOR_EACH(PairType p IN _wrapped.GetEnumerator())
				slot(CollectionOp::Added, p.Key, p.Value);
		}
	};

	/** @} */

}

#endif
