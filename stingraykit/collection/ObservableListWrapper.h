#ifndef STINGRAYKIT_COLLECTION_OBSERVABLELISTWRAPPER_H
#define STINGRAYKIT_COLLECTION_OBSERVABLELISTWRAPPER_H

// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/ForEach.h>
#include <stingraykit/collection/IObservableList.h>
#include <stingraykit/signal/signals.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename Wrapped_ >
	class ObservableListWrapper : public virtual IObservableList<typename Wrapped_::ValueType>
	{
		using ExternalMutexPointer = signal_policies::threading::ExternalMutexPointer;

	public:
		using ValueType = typename Wrapped_::ValueType;
		using ObservableInterface = IObservableList<ValueType>;
		using OnChangedSignature = typename ObservableInterface::OnChangedSignature;

	private:
		Wrapped_												_wrapped;
		shared_ptr<const Mutex>									_mutex;
		signal<OnChangedSignature, ExternalMutexPointer>		_onChanged;

	public:
		ObservableListWrapper()
			:	ObservableListWrapper(make_shared_ptr<Mutex>())
		{ }

		explicit ObservableListWrapper(const shared_ptr<const Mutex>& mutex)
			:	_mutex(STINGRAYKIT_REQUIRE_NOT_NULL(mutex)),
				_onChanged(ExternalMutexPointer(_mutex), Bind(&ObservableListWrapper::OnChangedPopulator, this, _1))
		{ }

		explicit ObservableListWrapper(const shared_ptr<IEnumerable<ValueType>>& enumerable)
			:	ObservableListWrapper(make_shared_ptr<Mutex>(), enumerable)
		{ }

		explicit ObservableListWrapper(const shared_ptr<IEnumerator<ValueType>>& enumerator)
			:	ObservableListWrapper(make_shared_ptr<Mutex>(), enumerator)
		{ }

		ObservableListWrapper(const shared_ptr<const Mutex>& mutex, const shared_ptr<IEnumerable<ValueType>>& enumerable)
			:	_wrapped(enumerable),
				_mutex(STINGRAYKIT_REQUIRE_NOT_NULL(mutex)),
				_onChanged(ExternalMutexPointer(_mutex), Bind(&ObservableListWrapper::OnChangedPopulator, this, _1))
		{ }

		ObservableListWrapper(const shared_ptr<const Mutex>& mutex, const shared_ptr<IEnumerator<ValueType>>& enumerator)
			:	_wrapped(enumerator),
				_mutex(STINGRAYKIT_REQUIRE_NOT_NULL(mutex)),
				_onChanged(ExternalMutexPointer(_mutex), Bind(&ObservableListWrapper::OnChangedPopulator, this, _1))
		{ }

		shared_ptr<IEnumerator<ValueType>> GetEnumerator() const override
		{
			signal_locker l(_onChanged);
			return _wrapped.GetEnumerator();
		}

		shared_ptr<IEnumerable<ValueType>> Reverse() const override
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

		bool Contains(const ValueType& value) const override
		{
			signal_locker l(_onChanged);
			return _wrapped.Contains(value);
		}

		optional<size_t> IndexOf(const ValueType& obj) const override
		{
			signal_locker l(_onChanged);
			return _wrapped.IndexOf(obj);
		}

		ValueType Get(size_t index) const override
		{
			signal_locker l(_onChanged);
			return _wrapped.Get(index);
		}

		bool TryGet(size_t index, ValueType& value) const override
		{
			signal_locker l(_onChanged);
			return _wrapped.TryGet(index, value);
		}

		void Add(const ValueType& value) override
		{
			signal_locker l(_onChanged);
			_wrapped.Add(value);
			_onChanged(CollectionOp::Added, _wrapped.GetCount() - 1, value);
		}

		void Set(size_t index, const ValueType& value) override
		{
			signal_locker l(_onChanged);
			_wrapped.Set(index, value);
			_onChanged(CollectionOp::Updated, index, value);
		}

		void Insert(size_t index, const ValueType& value) override
		{
			signal_locker l(_onChanged);
			_wrapped.Insert(index, value);
			_onChanged(CollectionOp::Added, index, value);
		}

		void RemoveAt(size_t index) override
		{
			signal_locker l(_onChanged);
			const ValueType value = _wrapped.Get(index);
			_wrapped.RemoveAt(index);
			_onChanged(CollectionOp::Removed, index, value);
		}

		bool Remove(const ValueType& value) override
		{
			signal_locker l(_onChanged);
			if (const optional<size_t> index = _wrapped.IndexOf(value))
			{
				RemoveAt(*index);
				return true;
			}
			return false;
		}

		size_t RemoveAll(const function<bool (const ValueType&)>& pred) override
		{
			signal_locker l(_onChanged);
			const size_t count = _wrapped.GetCount();
			size_t ret = 0;
			for (size_t index = 0; index < count; ++index)
			{
				const size_t realIndex = index - ret;
				const ValueType value = _wrapped.Get(realIndex);
				if (!pred(value))
					continue;

				_wrapped.RemoveAt(realIndex);
				_onChanged(CollectionOp::Removed, realIndex, value);
				++ret;
			}
			return ret;
		}

		void Clear() override
		{
			signal_locker l(_onChanged);
			while (!_wrapped.IsEmpty())
				RemoveAt(0);
		}

		signal_connector<OnChangedSignature> OnChanged() const override
		{ return _onChanged.connector(); }

		const Mutex& GetSyncRoot() const override
		{ return *_mutex; }

	private:
		void OnChangedPopulator(const function<OnChangedSignature>& slot) const
		{
			size_t i = 0;
			FOR_EACH(const ValueType value IN _wrapped.GetEnumerator())
				slot(CollectionOp::Added, i++, value);
		}
	};

	/** @} */

}

#endif
