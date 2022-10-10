#ifndef STINGRAYKIT_COLLECTION_TWOQUEUECACHE_H
#define STINGRAYKIT_COLLECTION_TWOQUEUECACHE_H

// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/QueueCache.h>

namespace stingray
{

	template < typename Key_, typename Value_, typename SizeMapper_ = DefaultCacheSizeMapper, typename Less_ = comparers::Less >
	class TwoQueueCache : public virtual ICache<Key_, Value_>
	{
		typedef ICache<Key_, Value_> Base;

		typedef typename Base::KeyPassingType KeyPassingType;
		typedef typename Base::ValuePassingType ValuePassingType;

		typedef typename Base::OnEvictedSignature OnEvictedSignature;

		typedef typename LruCache<Key_, Value_, SizeMapper_, Less_>::ValueT HotCache;
		typedef typename FifoCache<Key_, Value_, SizeMapper_, Less_>::ValueT Queue;

	private:
		Queue							_inQueue;
		Queue							_outQueue;
		HotCache						_hotCache;

		signal<OnEvictedSignature>		_onEvicted;

		ThreadlessTokenPool				_connections;

	public:
		TwoQueueCache(size_t inQueueCapacity, size_t outQueueCapacity, size_t hotCacheCapacity)
			:	_inQueue(inQueueCapacity),
				_outQueue(outQueueCapacity),
				_hotCache(hotCacheCapacity)
		{
			_connections += _inQueue.OnEvicted().connect(Bind(&Queue::Set, wrap_ref(_outQueue), _1, _2));
			_connections += _outQueue.OnEvicted().connect(_onEvicted.invoker());
			_connections += _hotCache.OnEvicted().connect(_onEvicted.invoker());
		}

		virtual bool TryGet(KeyPassingType key, Value_& out)
		{
			if (_hotCache.TryGet(key, out) || _inQueue.TryGet(key, out))
				return true;

			if (!_outQueue.TryGet(key, out))
				return false;

			_outQueue.TryRemove(key);
			_hotCache.Set(key, out);

			return true;
		}

		virtual void Set(KeyPassingType key, ValuePassingType value)
		{
			if (!_hotCache.TryRemove(key))
				_outQueue.TryRemove(key);

			_inQueue.Set(key, value);
		}

		virtual bool TryRemove(KeyPassingType key)
		{ return _hotCache.TryRemove(key) || _outQueue.TryRemove(key) || _inQueue.TryRemove(key); }

		virtual void Clear()
		{
			_hotCache.Clear();
			_outQueue.Clear();
			_inQueue.Clear();
		}

		virtual size_t GetSize() const
		{ return _hotCache.GetSize() + _inQueue.GetSize() + _outQueue.GetSize(); }

		virtual signal_connector<OnEvictedSignature> OnEvicted() const
		{ return _onEvicted.connector(); }
	};

}

#endif
