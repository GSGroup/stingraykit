#ifndef STINGRAYKIT_COLLECTION_MULTIMAPOBSERVABLEDICTIONARY_H
#define STINGRAYKIT_COLLECTION_MULTIMAPOBSERVABLEDICTIONARY_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
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
	class MultiMapObservableDictionary : public virtual IObservableMultiDictionary<KeyType_, ValueType_>
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
		Wrapped													_wrapped;
		shared_ptr<const Mutex>									_mutex;
		signal<OnChangedSignature, ExternalMutexPointer>		_onChanged;

	public:
		MultiMapObservableDictionary()
			:	MultiMapObservableDictionary(make_shared_ptr<Mutex>())
		{ }

		explicit MultiMapObservableDictionary(const shared_ptr<const Mutex>& mutex)
			:	_mutex(STINGRAYKIT_REQUIRE_NOT_NULL(mutex)),
				_onChanged(ExternalMutexPointer(_mutex), Bind(&MultiMapObservableDictionary::OnChangedPopulator, this, _1))
		{ }

		explicit MultiMapObservableDictionary(const shared_ptr<IEnumerable<PairType>>& enumerable)
			:	MultiMapObservableDictionary(make_shared_ptr<Mutex>(), enumerable)
		{ }

		explicit MultiMapObservableDictionary(const shared_ptr<IEnumerator<PairType>>& enumerator)
			:	MultiMapObservableDictionary(make_shared_ptr<Mutex>(), enumerator)
		{ }

		MultiMapObservableDictionary(const shared_ptr<const Mutex>& mutex, const shared_ptr<IEnumerable<PairType>>& enumerable)
			:	_wrapped(enumerable),
				_mutex(STINGRAYKIT_REQUIRE_NOT_NULL(mutex)),
				_onChanged(ExternalMutexPointer(_mutex), Bind(&MultiMapObservableDictionary::OnChangedPopulator, this, _1))
		{ }

		MultiMapObservableDictionary(const shared_ptr<const Mutex>& mutex, const shared_ptr<IEnumerator<PairType>>& enumerator)
			:	_wrapped(enumerator),
				_mutex(STINGRAYKIT_REQUIRE_NOT_NULL(mutex)),
				_onChanged(ExternalMutexPointer(_mutex), Bind(&MultiMapObservableDictionary::OnChangedPopulator, this, _1))
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

		size_t CountKey(const KeyType& key) const override
		{
			signal_locker l(_onChanged);
			return _wrapped.CountKey(key);
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

		ValueType GetFirst(const KeyType& key) const override
		{
			signal_locker l(_onChanged);
			return _wrapped.GetFirst(key);
		}

		bool TryGetFirst(const KeyType& key, ValueType& outValue) const override
		{
			signal_locker l(_onChanged);
			return _wrapped.TryGetFirst(key, outValue);
		}

		shared_ptr<IEnumerator<ValueType>> GetAll(const KeyType& key) const override
		{
			signal_locker l(_onChanged);
			return _wrapped.GetAll(key);
		}

		void Add(const KeyType& key, const ValueType& value) override
		{
			signal_locker l(_onChanged);
			_wrapped.Add(key, value);
			_onChanged(CollectionOp::Added, key, value);
		}

		bool RemoveFirst(const KeyType& key, const optional<ValueType>& value_ = null) override
		{
			signal_locker l(_onChanged);

			optional<ValueType> toRemove;
			FOR_EACH(const ValueType value IN _wrapped.GetAll(key))
			{
				if (value_ && !ValueCompareType_()(*value_, value))
					continue;

				toRemove = value;
				break;
			}

			if (!toRemove)
				return false;

			_wrapped.RemoveFirst(key, toRemove);
			_onChanged(CollectionOp::Removed, key, *toRemove);
			return true;
		}

		size_t RemoveAll(const KeyType& key) override
		{
			signal_locker l(_onChanged);
			size_t ret = 0;
			FOR_EACH(const PairType pair IN _wrapped.Find(key))
			{
				if (KeyCompareType_()(pair.Key, key) || KeyCompareType_()(key, pair.Key))
					break;

				_wrapped.RemoveFirst(pair.Key, pair.Value);
				_onChanged(CollectionOp::Removed, pair.Key, pair.Value);
				++ret;
			}
			return ret;
		}

		size_t RemoveWhere(const function<bool (const KeyType&, const ValueType&)>& pred) override
		{
			signal_locker l(_onChanged);
			size_t ret = 0;
			FOR_EACH(const PairType pair IN _wrapped.GetEnumerator() WHERE pred(pair.Key, pair.Value))
			{
				_wrapped.RemoveFirst(pair.Key, pair.Value);
				_onChanged(CollectionOp::Removed, pair.Key, pair.Value);
				++ret;
			}
			return ret;
		}

		void Clear() override
		{
			signal_locker l(_onChanged);
			FOR_EACH(const PairType pair IN _wrapped.GetEnumerator())
			{
				_wrapped.RemoveFirst(pair.Key, pair.Value);
				_onChanged(CollectionOp::Removed, pair.Key, pair.Value);
			}
		}

		signal_connector<OnChangedSignature> OnChanged() const override
		{ return _onChanged.connector(); }

		const Mutex& GetSyncRoot() const override
		{ return *_mutex; }

	private:
		void OnChangedPopulator(const function<OnChangedSignature>& slot) const
		{
			FOR_EACH(const PairType pair IN _wrapped.GetEnumerator())
				slot(CollectionOp::Added, pair.Key, pair.Value);
		}
	};

	/** @} */

}

#endif
