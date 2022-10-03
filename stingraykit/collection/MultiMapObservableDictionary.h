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
	class MultiMapObservableDictionary
		:	public MultiMapDictionary<KeyType_, ValueType_, KeyCompareType_, ValueCompareType_>,
			public virtual IObservableMultiDictionary<KeyType_, ValueType_>
	{
		using Wrapped = MultiMapDictionary<KeyType_, ValueType_, KeyCompareType_, ValueCompareType_>;

		using ExternalMutexPointer = signal_policies::threading::ExternalMutexPointer;

	public:
		using KeyType = typename Wrapped::KeyType;
		using ValueType = typename Wrapped::ValueType;
		using PairType = typename Wrapped::PairType;
		using ObservableInterface = IObservableMultiDictionary<KeyType, ValueType>;
		using OnChangedSignature = typename ObservableInterface::OnChangedSignature;

	private:
		shared_ptr<Mutex>									_mutex;
		signal<OnChangedSignature, ExternalMutexPointer>	_onChanged;

	public:
		MultiMapObservableDictionary()
			:	Wrapped(),
				_mutex(make_shared_ptr<Mutex>()),
				_onChanged(ExternalMutexPointer(_mutex), Bind(&MultiMapObservableDictionary::OnChangedPopulator, this, _1))
		{ }

		explicit MultiMapObservableDictionary(const shared_ptr<IEnumerable<PairType>>& enumerable)
			:	Wrapped(enumerable),
				_mutex(make_shared_ptr<Mutex>()),
				_onChanged(ExternalMutexPointer(_mutex), Bind(&MultiMapObservableDictionary::OnChangedPopulator, this, _1))
		{ }

		explicit MultiMapObservableDictionary(const shared_ptr<IEnumerator<PairType>>& enumerator)
			:	Wrapped(enumerator),
				_mutex(make_shared_ptr<Mutex>()),
				_onChanged(ExternalMutexPointer(_mutex), Bind(&MultiMapObservableDictionary::OnChangedPopulator, this, _1))
		{ }

		shared_ptr<IEnumerator<PairType>> GetEnumerator() const override
		{
			signal_locker l(_onChanged);
			return Wrapped::GetEnumerator();
		}

		shared_ptr<IEnumerable<PairType>> Reverse() const override
		{
			signal_locker l(_onChanged);
			return Wrapped::Reverse();
		}

		size_t GetCount() const override
		{
			signal_locker l(_onChanged);
			return Wrapped::GetCount();
		}

		bool IsEmpty() const override
		{
			signal_locker l(_onChanged);
			return Wrapped::IsEmpty();
		}

		bool ContainsKey(const KeyType& key) const override
		{
			signal_locker l(_onChanged);
			return Wrapped::ContainsKey(key);
		}

		size_t CountKey(const KeyType& key) const override
		{
			signal_locker l(_onChanged);
			return Wrapped::CountKey(key);
		}

		shared_ptr<IEnumerator<PairType>> Find(const KeyType& key) const override
		{
			signal_locker l(_onChanged);
			return Wrapped::Find(key);
		}

		shared_ptr<IEnumerator<PairType>> ReverseFind(const KeyType& key) const override
		{
			signal_locker l(_onChanged);
			return Wrapped::ReverseFind(key);
		}

		ValueType GetFirst(const KeyType& key) const override
		{
			signal_locker l(_onChanged);
			return Wrapped::GetFirst(key);
		}

		bool TryGetFirst(const KeyType& key, ValueType& outValue) const override
		{
			signal_locker l(_onChanged);
			return Wrapped::TryGetFirst(key, outValue);
		}

		shared_ptr<IEnumerator<ValueType>> GetAll(const KeyType& key) const override
		{
			signal_locker l(_onChanged);
			return Wrapped::GetAll(key);
		}

		void Add(const KeyType& key, const ValueType& value) override
		{
			signal_locker l(_onChanged);
			Wrapped::Add(key, value);
			_onChanged(CollectionOp::Added, key, value);
		}

		void RemoveFirst(const KeyType& key, const optional<ValueType>& value = null) override
		{ MultiMapObservableDictionary::TryRemoveFirst(key, value); }

		bool TryRemoveFirst(const KeyType& key, const optional<ValueType>& value_ = null) override
		{
			signal_locker l(_onChanged);
			FOR_EACH(ValueType value IN Wrapped::GetAll(key))
			{
				if (value_ && !ValueCompareType_()(*value_, value))
					continue;

				Wrapped::RemoveFirst(key, value);
				_onChanged(CollectionOp::Removed, key, value);
				return true;
			}
			return false;
		}

		size_t RemoveAll(const KeyType& key) override
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

		size_t RemoveWhere(const function<bool (const KeyType&, const ValueType&)>& pred) override
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

		void Clear() override
		{
			signal_locker l(_onChanged);
			FOR_EACH(PairType v IN Wrapped::GetEnumerator())
			{
				Wrapped::RemoveFirst(v.Key, v.Value);
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
			FOR_EACH(PairType p IN Wrapped::GetEnumerator())
				slot(CollectionOp::Added, p.Key, p.Value);
		}
	};

	/** @} */

}

#endif
