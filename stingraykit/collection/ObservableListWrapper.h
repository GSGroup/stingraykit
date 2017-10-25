#ifndef STINGRAYKIT_COLLECTION_OBSERVABLELISTWRAPPER_H
#define STINGRAYKIT_COLLECTION_OBSERVABLELISTWRAPPER_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
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
	struct ObservableListWrapper
		:	public Wrapped_,
			public virtual IObservableList<typename Wrapped_::ValueType>
	{
		typedef signal_policies::threading::ExternalMutexPointer ExternalMutexPointer;

	public:
		typedef typename Wrapped_::ValueType						ValueType;
		typedef IObservableList<ValueType>							ObservableInterface;
		typedef typename ObservableInterface::OnChangedSignature	OnChangedSignature;

	private:
		shared_ptr<Mutex>									_mutex;
		signal<OnChangedSignature, ExternalMutexPointer>	_onChanged;

	public:
		ObservableListWrapper()
			:	Wrapped_(),
				_mutex(make_shared<Mutex>()),
				_onChanged(ExternalMutexPointer(_mutex), bind(&ObservableListWrapper::OnChangedPopulator, this, _1))
		{ }

		ObservableListWrapper(shared_ptr<IEnumerator<ValueType> > enumerator)
			:	Wrapped_(enumerator),
				_mutex(make_shared<Mutex>()),
				_onChanged(ExternalMutexPointer(_mutex), bind(&ObservableListWrapper::OnChangedPopulator, this, _1))
		{ }

		ObservableListWrapper(shared_ptr<IEnumerable<ValueType> > enumerable)
			:	Wrapped_(enumerable),
				_mutex(make_shared<Mutex>()),
				_onChanged(ExternalMutexPointer(_mutex), bind(&ObservableListWrapper::OnChangedPopulator, this, _1))
		{ }

		virtual shared_ptr<IEnumerator<ValueType> > GetEnumerator() const
		{
			signal_locker l(_onChanged);
			return Wrapped_::GetEnumerator();
		}

		virtual shared_ptr<IEnumerable<ValueType> > Reverse() const
		{
			signal_locker l(_onChanged);
			return Wrapped_::Reverse();
		}

		virtual size_t GetCount() const
		{
			signal_locker l(_onChanged);
			return Wrapped_::GetCount();
		}

		virtual bool Contains(const ValueType& value) const
		{
			signal_locker l(_onChanged);
			return Wrapped_::Contains(value);
		}

		virtual optional<size_t> IndexOf(const ValueType& obj) const
		{
			signal_locker l(_onChanged);
			return Wrapped_::IndexOf(obj);
		}

		virtual ValueType Get(size_t index) const
		{
			signal_locker l(_onChanged);
			return Wrapped_::Get(index);
		}

		virtual bool TryGet(size_t index, ValueType& value) const
		{
			signal_locker l(_onChanged);
			return Wrapped_::TryGet(index, value);
		}

		virtual void Add(const ValueType& value)
		{
			signal_locker l(_onChanged);
			Wrapped_::Add(value);
			_onChanged(CollectionOp::Added, Wrapped_::GetCount() - 1, value);
		}

		virtual void Set(size_t index, const ValueType& value)
		{
			signal_locker l(_onChanged);
			Wrapped_::Set(index, value);
			_onChanged(CollectionOp::Updated, index, value);
		}

		virtual void Insert(size_t index, const ValueType& value)
		{
			signal_locker l(_onChanged);
			Wrapped_::Insert(index, value);
			_onChanged(CollectionOp::Added, index, value);
		}

		virtual void Remove(const ValueType& value)
		{
			signal_locker l(_onChanged);
			if (const optional<size_t> index = Wrapped_::IndexOf(value))
				RemoveAt(*index);
		}

		virtual void RemoveAt(size_t index)
		{
			signal_locker l(_onChanged);
			ValueType value = Get(index);
			Wrapped_::RemoveAt(index);
			_onChanged(CollectionOp::Removed, index, value);
		}

		virtual size_t RemoveAll(const function<bool (const ValueType&)>& pred)
		{
			signal_locker l(_onChanged);
			const size_t count = GetCount();
			size_t ret = 0;
			for (size_t index = 0; index < count; ++index)
			{
				const size_t realIndex = index - ret;
				const ValueType value = Get(realIndex);
				if (!pred(value))
					continue;

				Wrapped_::RemoveAt(realIndex);
				_onChanged(CollectionOp::Removed, realIndex, value);
				++ret;
			}
			return ret;
		}

		virtual void Clear()
		{
			signal_locker l(_onChanged);
			while (!this->IsEmpty())
				RemoveAt(0);
		}

		virtual signal_connector<OnChangedSignature> OnChanged() const
		{ return _onChanged.connector(); }

		virtual const Mutex& GetSyncRoot() const
		{ return *_mutex; }

	private:
		virtual void OnChangedPopulator(const function<OnChangedSignature>& slot)
		{
			size_t i = 0;
			FOR_EACH(ValueType v IN this->GetEnumerator())
				slot(CollectionOp::Added, i++, v);
		}
	};

	/** @} */

}

#endif
