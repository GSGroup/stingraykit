#ifndef STINGRAYKIT_COLLECTION_SORTEDOBSERVABLEMULTISET_H
#define STINGRAYKIT_COLLECTION_SORTEDOBSERVABLEMULTISET_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/ForEach.h>
#include <stingraykit/collection/IObservableMultiSet.h>
#include <stingraykit/collection/SortedMultiSet.h>
#include <stingraykit/signal/signals.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename ValueType_, typename CompareType_ = comparers::Less >
	struct SortedObservableMultiSet
		:	public SortedMultiSet<ValueType_, CompareType_>,
			public virtual IObservableMultiSet<ValueType_>
	{
		typedef SortedMultiSet<ValueType_, CompareType_> Wrapped;

		typedef signal_policies::threading::ExternalMutexPointer ExternalMutexPointer;

	public:
		typedef typename Wrapped::ValueType							ValueType;
		typedef IObservableMultiSet<ValueType>						ObservableInterface;
		typedef typename ObservableInterface::OnChangedSignature	OnChangedSignature;

	private:
		shared_ptr<Mutex>										_mutex;
		signal<OnChangedSignature, ExternalMutexPointer>		_onChanged;

	public:
		SortedObservableMultiSet()
			:	Wrapped(),
				_mutex(make_shared<Mutex>()),
				_onChanged(ExternalMutexPointer(_mutex), bind(&SortedObservableMultiSet::OnChangedPopulator, this, _1))
		{ }

		SortedObservableMultiSet(shared_ptr<IEnumerator<ValueType> > enumerator)
			:	Wrapped(enumerator),
				_mutex(make_shared<Mutex>()),
				_onChanged(ExternalMutexPointer(_mutex), bind(&SortedObservableMultiSet::OnChangedPopulator, this, _1))
		{ }

		SortedObservableMultiSet(shared_ptr<IEnumerable<ValueType> > enumerable)
			:	Wrapped(enumerable),
				_mutex(make_shared<Mutex>()),
				_onChanged(ExternalMutexPointer(_mutex), bind(&SortedObservableMultiSet::OnChangedPopulator, this, _1))
		{ }

		virtual shared_ptr<IEnumerator<ValueType> > GetEnumerator() const
		{
			signal_locker l(_onChanged);
			return Wrapped::GetEnumerator();
		}

		virtual shared_ptr<IEnumerable<ValueType> > Reverse() const
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

		virtual bool Contains(const ValueType& value) const
		{
			signal_locker l(_onChanged);
			return Wrapped::Contains(value);
		}

		virtual size_t Count(const ValueType& value) const
		{
			signal_locker l(_onChanged);
			return Wrapped::Count(value);
		}

		virtual shared_ptr<IEnumerator<ValueType> > Find(const ValueType& value) const
		{
			signal_locker l(_onChanged);
			return Wrapped::Find(value);
		}

		virtual shared_ptr<IEnumerator<ValueType> > ReverseFind(const ValueType& value) const
		{
			signal_locker l(_onChanged);
			return Wrapped::ReverseFind(value);
		}

		virtual void Add(const ValueType& value)
		{
			signal_locker l(_onChanged);
			Wrapped::Add(value);
			_onChanged(CollectionOp::Added, value);
		}

		virtual void RemoveFirst(const ValueType& value)
		{ SortedObservableMultiSet::TryRemoveFirst(value); }

		virtual bool TryRemoveFirst(const ValueType& value)
		{
			signal_locker l(_onChanged);
			FOR_EACH(ValueType v IN Wrapped::Find(value))
			{
				Wrapped::RemoveFirst(v);
				_onChanged(CollectionOp::Removed, v);
				return true;
			}
			return false;
		}

		virtual size_t RemoveAll(const ValueType& value)
		{
			signal_locker l(_onChanged);
			size_t ret = 0;
			FOR_EACH(ValueType v IN Wrapped::Find(value))
			{
				if (CompareType_()(v, value) || CompareType_()(value, v))
					break;

				Wrapped::RemoveFirst(v);
				_onChanged(CollectionOp::Removed, v);
				++ret;
			}
			return ret;
		}

		virtual size_t RemoveWhere(const function<bool (const ValueType&)>& pred)
		{
			signal_locker l(_onChanged);
			size_t ret = 0;
			FOR_EACH(ValueType v IN Wrapped::GetEnumerator() WHERE pred(v))
			{
				Wrapped::RemoveFirst(v);
				_onChanged(CollectionOp::Removed, v);
				++ret;
			}
			return ret;
		}

		virtual void Clear()
		{
			signal_locker l(_onChanged);
			FOR_EACH(ValueType v IN Wrapped::GetEnumerator())
			{
				Wrapped::RemoveFirst(v);
				_onChanged(CollectionOp::Removed, v);
			}
		}

		virtual signal_connector<OnChangedSignature> OnChanged() const
		{ return _onChanged.connector(); }

		virtual const Mutex& GetSyncRoot() const
		{ return *_mutex; }

	private:
		void OnChangedPopulator(const function<OnChangedSignature>& slot) const
		{
			FOR_EACH(ValueType v IN Wrapped::GetEnumerator())
				slot(CollectionOp::Added, v);
		}
	};

	/** @} */

}

#endif
