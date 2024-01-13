#ifndef STINGRAYKIT_COLLECTION_QUEUECACHE_H
#define STINGRAYKIT_COLLECTION_QUEUECACHE_H

// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/DefaultCacheSizeMapper.h>
#include <stingraykit/collection/ICache.h>
#include <stingraykit/compare/comparers.h>
#include <stingraykit/signal/signals.h>

#include <map>

namespace stingray
{

	struct QueueEvictionPolicy
	{
		STINGRAYKIT_ENUM_VALUES(Fifo, Lru);
		STINGRAYKIT_DECLARE_ENUM_CLASS(QueueEvictionPolicy);
	};


	template < typename Key_, typename Value_, QueueEvictionPolicy::Enum EvictionPolicy_, typename SizeMapper_ = DefaultCacheSizeMapper, typename Less_ = comparers::Less >
	class QueueCache final : public virtual ICache<Key_, Value_>
	{
		static_assert(comparers::IsRelationalComparer<Less_>::Value, "Expected Relational comparer");

		using Base = ICache<Key_, Value_>;

		using KeyPassingType = typename Base::KeyPassingType;
		using ValuePassingType = typename Base::ValuePassingType;

		using OnEvictedSignature = typename Base::OnEvictedSignature;

		using StampType = u64;

		struct StampedValue
		{
			Value_		Value;
			StampType	Stamp;

		public:
			StampedValue(ValuePassingType value, StampType stamp)
				:	Value(value),
					Stamp(stamp)
			{ }
		};

		using Dictionary = std::map<Key_, StampedValue, Less_>;
		using Queue = std::map<StampType, typename Dictionary::iterator>;

	private:
		size_t							_capacity;
		size_t							_size;
		SizeMapper_						_sizeMapper;

		StampType						_monotonic;

		Dictionary						_dictionary;
		Queue							_queue;

		signal<OnEvictedSignature>		_onEvicted;

	public:
		QueueCache(size_t capacity)
			:	_capacity(capacity),
				_size(),
				_monotonic()
		{ }

		bool TryGet(KeyPassingType key, Value_& out) override
		{ return DoTryGet<EvictionPolicy_>(key, out); }

		void Set(KeyPassingType key, ValuePassingType value) override
		{
			const StampType newStamp = MakeStamp();

			typename Dictionary::iterator dictionaryIter = _dictionary.find(key);
			if (dictionaryIter != _dictionary.end())
			{
				StampedValue& stampedValue = dictionaryIter->second;

				_size -= _sizeMapper(stampedValue.Value);
				_queue.erase(stampedValue.Stamp);
				stampedValue = StampedValue(value, newStamp);
			}
			else
				dictionaryIter = _dictionary.emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(value, newStamp)).first;

			_queue.emplace(newStamp, dictionaryIter);
			_size += _sizeMapper(value);

			EvictExpired();
		}

		bool Remove(KeyPassingType key) override
		{
			const typename Dictionary::iterator dictionaryIter = _dictionary.find(key);
			if (dictionaryIter == _dictionary.end())
				return false;

			const typename Queue::iterator queueIter = _queue.find(dictionaryIter->second.Stamp);
			STINGRAYKIT_CHECK(queueIter != _queue.end(), "No corresponding queue entry for existing dictionary entry.");

			DoRemove(dictionaryIter, queueIter);
			return true;
		}

		void Clear() override
		{
			_size = 0;
			_queue.clear();
			_dictionary.clear();
			_monotonic = 0;
		}

		size_t GetSize() const override
		{ return _size; }

		signal_connector<OnEvictedSignature> OnEvicted() const override
		{ return _onEvicted.connector(); }

	private:
		template < QueueEvictionPolicy::Enum EvictionPolicy__ >
		typename EnableIf<EvictionPolicy__ == QueueEvictionPolicy::Fifo, bool>::ValueT DoTryGet(KeyPassingType key, Value_& out)
		{
			const typename Dictionary::iterator iter = _dictionary.find(key);
			if (iter == _dictionary.end())
				return false;

			out = iter->second.Value;
			return true;
		}

		template < QueueEvictionPolicy::Enum EvictionPolicy__ >
		typename EnableIf<EvictionPolicy__ == QueueEvictionPolicy::Lru, bool>::ValueT DoTryGet(KeyPassingType key, Value_& out)
		{
			const typename Dictionary::iterator iter = _dictionary.find(key);
			if (iter == _dictionary.end())
				return false;

			StampedValue& value = iter->second;

			const StampType newStamp = MakeStamp();

			_queue.erase(value.Stamp);
			_queue.emplace(newStamp, iter);
			value.Stamp = newStamp;

			out = value.Value;
			return true;
		}

		StampType MakeStamp()
		{ return _monotonic++; }

		void EvictExpired()
		{
			while (_size > _capacity)
			{
				STINGRAYKIT_CHECK(!_queue.empty(), StringBuilder() % "Size limit reached, but the queue is empty. Size: " % _size % ", capacity: " % _capacity);

				const typename Queue::iterator queueIter = _queue.begin();
				const typename Dictionary::iterator dictionaryIter = queueIter->second;

				const Key_ key = dictionaryIter->first;
				const Value_ value = dictionaryIter->second.Value;

				DoRemove(dictionaryIter, queueIter);

				_onEvicted(key, value);
			}
		}

		void DoRemove(typename Dictionary::iterator dictionaryIter, typename Queue::iterator queueIter)
		{
			_size -= _sizeMapper(dictionaryIter->second.Value);
			_queue.erase(queueIter);
			_dictionary.erase(dictionaryIter);
		}
	};


	template < typename Key_, typename Value_, typename SizeMapper_ = DefaultCacheSizeMapper, typename Less_ = comparers::Less >
	using FifoCache = QueueCache<Key_, Value_, QueueEvictionPolicy::Fifo, SizeMapper_, Less_>;


	template < typename Key_, typename Value_, typename SizeMapper_ = DefaultCacheSizeMapper, typename Less_ = comparers::Less >
	using LruCache = QueueCache<Key_, Value_, QueueEvictionPolicy::Lru, SizeMapper_, Less_>;

}

#endif
