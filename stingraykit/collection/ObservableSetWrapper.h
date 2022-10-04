#ifndef STINGRAYKIT_COLLECTION_OBSERVABLESETWRAPPER_H
#define STINGRAYKIT_COLLECTION_OBSERVABLESETWRAPPER_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/ForEach.h>
#include <stingraykit/collection/IObservableSet.h>
#include <stingraykit/signal/signals.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename Wrapped_ >
	class ObservableSetWrapper : public virtual IObservableSet<typename Wrapped_::ValueType>
	{
		using ExternalMutexPointer = signal_policies::threading::ExternalMutexPointer;

	public:
		using ValueType = typename Wrapped_::ValueType;
		using ObservableInterface = IObservableSet<ValueType>;
		using OnChangedSignature = typename ObservableInterface::OnChangedSignature;

	private:
		Wrapped_												_wrapped;
		shared_ptr<Mutex>										_mutex;
		signal<OnChangedSignature, ExternalMutexPointer>		_onChanged;

	public:
		ObservableSetWrapper()
			:	_mutex(make_shared_ptr<Mutex>()),
				_onChanged(ExternalMutexPointer(_mutex), Bind(&ObservableSetWrapper::OnChangedPopulator, this, _1))
		{ }

		explicit ObservableSetWrapper(const shared_ptr<IEnumerable<ValueType>>& enumerable)
			:	_wrapped(enumerable),
				_mutex(make_shared_ptr<Mutex>()),
				_onChanged(ExternalMutexPointer(_mutex), Bind(&ObservableSetWrapper::OnChangedPopulator, this, _1))
		{ }

		explicit ObservableSetWrapper(const shared_ptr<IEnumerator<ValueType>>& enumerator)
			:	_wrapped(enumerator),
				_mutex(make_shared_ptr<Mutex>()),
				_onChanged(ExternalMutexPointer(_mutex), Bind(&ObservableSetWrapper::OnChangedPopulator, this, _1))
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

		shared_ptr<IEnumerator<ValueType>> Find(const ValueType& value) const override
		{
			signal_locker l(_onChanged);
			return _wrapped.Find(value);
		}

		shared_ptr<IEnumerator<ValueType>> ReverseFind(const ValueType& value) const override
		{
			signal_locker l(_onChanged);
			return _wrapped.ReverseFind(value);
		}

		void Add(const ValueType& value) override
		{
			signal_locker l(_onChanged);
			bool signal = !_wrapped.Contains(value);
			_wrapped.Add(value);
			if (signal)
				_onChanged(CollectionOp::Added, value);
		}

		void Remove(const ValueType& value) override
		{ TryRemove(value); }

		bool TryRemove(const ValueType& value) override
		{
			signal_locker l(_onChanged);
			if (!_wrapped.TryRemove(value))
				return false;

			_onChanged(CollectionOp::Removed, value);
			return true;
		}

		size_t RemoveWhere(const function<bool (const ValueType&)>& pred) override
		{
			signal_locker l(_onChanged);
			size_t ret = 0;
			FOR_EACH(const ValueType value IN _wrapped.GetEnumerator() WHERE pred(value))
			{
				_wrapped.Remove(value);
				_onChanged(CollectionOp::Removed, value);
				++ret;
			}
			return ret;
		}

		void Clear() override
		{
			signal_locker l(_onChanged);
			FOR_EACH(const ValueType value IN _wrapped.GetEnumerator())
			{
				_wrapped.Remove(value);
				_onChanged(CollectionOp::Removed, value);
			}
		}

		signal_connector<OnChangedSignature> OnChanged() const override
		{ return _onChanged.connector(); }

		const Mutex& GetSyncRoot() const override
		{ return *_mutex; }

	private:
		void OnChangedPopulator(const function<OnChangedSignature>& slot) const
		{
			FOR_EACH(const ValueType value IN _wrapped.GetEnumerator())
				slot(CollectionOp::Added, value);
		}
	};

	/** @} */

}

#endif
