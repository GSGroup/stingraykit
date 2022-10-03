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
	class ObservableDictionaryWrapper
		:	public Wrapped_,
			public virtual IObservableDictionary<typename Wrapped_::KeyType, typename Wrapped_::ValueType>
	{
		using ExternalMutexPointer = signal_policies::threading::ExternalMutexPointer;

	public:
		using KeyType = typename Wrapped_::KeyType;
		using ValueType = typename Wrapped_::ValueType;
		using PairType = typename Wrapped_::PairType;
		using ObservableInterface = IObservableDictionary<KeyType, ValueType>;
		using OnChangedSignature = typename ObservableInterface::OnChangedSignature;

	private:
		shared_ptr<Mutex>									_mutex;
		signal<OnChangedSignature, ExternalMutexPointer>	_onChanged;

	public:
		ObservableDictionaryWrapper()
			:	Wrapped_(),
				_mutex(make_shared_ptr<Mutex>()),
				_onChanged(ExternalMutexPointer(_mutex), Bind(&ObservableDictionaryWrapper::OnChangedPopulator, this, _1))
		{ }

		explicit ObservableDictionaryWrapper(const shared_ptr<IEnumerable<PairType>>& enumerable)
			:	Wrapped_(enumerable),
				_mutex(make_shared_ptr<Mutex>()),
				_onChanged(ExternalMutexPointer(_mutex), Bind(&ObservableDictionaryWrapper::OnChangedPopulator, this, _1))
		{ }

		explicit ObservableDictionaryWrapper(const shared_ptr<IEnumerator<PairType>>& enumerator)
			:	Wrapped_(enumerator),
				_mutex(make_shared_ptr<Mutex>()),
				_onChanged(ExternalMutexPointer(_mutex), Bind(&ObservableDictionaryWrapper::OnChangedPopulator, this, _1))
		{ }

		shared_ptr<IEnumerator<PairType>> GetEnumerator() const override
		{
			signal_locker l(_onChanged);
			return Wrapped_::GetEnumerator();
		}

		shared_ptr<IEnumerable<PairType>> Reverse() const override
		{
			signal_locker l(_onChanged);
			return Wrapped_::Reverse();
		}

		size_t GetCount() const override
		{
			signal_locker l(_onChanged);
			return Wrapped_::GetCount();
		}

		bool IsEmpty() const override
		{
			signal_locker l(_onChanged);
			return Wrapped_::IsEmpty();
		}

		bool ContainsKey(const KeyType& key) const override
		{
			signal_locker l(_onChanged);
			return Wrapped_::ContainsKey(key);
		}

		shared_ptr<IEnumerator<PairType>> Find(const KeyType& key) const override
		{
			signal_locker l(_onChanged);
			return Wrapped_::Find(key);
		}

		shared_ptr<IEnumerator<PairType>> ReverseFind(const KeyType& key) const override
		{
			signal_locker l(_onChanged);
			return Wrapped_::ReverseFind(key);
		}

		ValueType Get(const KeyType& key) const override
		{
			signal_locker l(_onChanged);
			return Wrapped_::Get(key);
		}

		bool TryGet(const KeyType& key, ValueType& outValue) const override
		{
			signal_locker l(_onChanged);
			return Wrapped_::TryGet(key, outValue);
		}

		void Set(const KeyType& key, const ValueType& value) override
		{
			signal_locker l(_onChanged);
			bool update = Wrapped_::ContainsKey(key);
			Wrapped_::Set(key, value);
			_onChanged(update ? CollectionOp::Updated : CollectionOp::Added, key, value);
		}

		void Remove(const KeyType& key) override
		{ TryRemove(key); }

		bool TryRemove(const KeyType& key) override
		{
			signal_locker l(_onChanged);
			ValueType value;
			if (!Wrapped_::TryGet(key, value))
				return false;

			Wrapped_::Remove(key);
			_onChanged(CollectionOp::Removed, key, value);
			return true;
		}

		size_t RemoveWhere(const function<bool (const KeyType&, const ValueType&)>& pred) override
		{
			signal_locker l(_onChanged);
			size_t ret = 0;
			FOR_EACH(PairType v IN Wrapped_::GetEnumerator() WHERE pred(v.Key, v.Value))
			{
				Wrapped_::Remove(v.Key);
				_onChanged(CollectionOp::Removed, v.Key, v.Value);
				++ret;
			}
			return ret;
		}

		void Clear() override
		{
			signal_locker l(_onChanged);
			FOR_EACH(PairType v IN Wrapped_::GetEnumerator())
			{
				Wrapped_::Remove(v.Key);
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
			FOR_EACH(PairType p IN Wrapped_::GetEnumerator())
				slot(CollectionOp::Added, p.Key, p.Value);
		}
	};

	/** @} */

}

#endif
