#ifndef STINGRAYKIT_COLLECTION_OBSERVABLESETWRAPPER_H
#define STINGRAYKIT_COLLECTION_OBSERVABLESETWRAPPER_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
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
	class ObservableSetWrapper
		:	public Wrapped_,
			public virtual IObservableSet<typename Wrapped_::ValueType>
	{
		typedef signal_policies::threading::ExternalMutexPointer ExternalMutexPointer;

	public:
		typedef typename Wrapped_::ValueType						ValueType;
		typedef IObservableSet<ValueType>							ObservableInterface;
		typedef typename ObservableInterface::OnChangedSignature	OnChangedSignature;

	private:
		shared_ptr<Mutex>										_mutex;
		signal<OnChangedSignature, ExternalMutexPointer>		_onChanged;

	public:
		ObservableSetWrapper()
			:	Wrapped_(),
				_mutex(make_shared<Mutex>()),
				_onChanged(ExternalMutexPointer(_mutex), bind(&ObservableSetWrapper::OnChangedPopulator, this, _1))
		{ }

		ObservableSetWrapper(shared_ptr<IEnumerator<ValueType> > enumerator)
			:	Wrapped_(enumerator),
				_mutex(make_shared<Mutex>()),
				_onChanged(ExternalMutexPointer(_mutex), bind(&ObservableSetWrapper::OnChangedPopulator, this, _1))
		{ }

		ObservableSetWrapper(shared_ptr<IEnumerable<ValueType> > enumerable)
			:	Wrapped_(enumerable),
				_mutex(make_shared<Mutex>()),
				_onChanged(ExternalMutexPointer(_mutex), bind(&ObservableSetWrapper::OnChangedPopulator, this, _1))
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

		virtual bool IsEmpty() const
		{
			signal_locker l(_onChanged);
			return Wrapped_::IsEmpty();
		}

		virtual bool Contains(const ValueType& value) const
		{
			signal_locker l(_onChanged);
			return Wrapped_::Contains(value);
		}

		virtual shared_ptr<IEnumerator<ValueType> > Find(const ValueType& value) const
		{
			signal_locker l(_onChanged);
			return Wrapped_::Find(value);
		}

		virtual shared_ptr<IEnumerator<ValueType> > ReverseFind(const ValueType& value) const
		{
			signal_locker l(_onChanged);
			return Wrapped_::ReverseFind(value);
		}

		virtual void Add(const ValueType& value)
		{
			signal_locker l(_onChanged);
			bool signal = !Wrapped_::Contains(value);
			Wrapped_::Add(value);
			if (signal)
				_onChanged(CollectionOp::Added, value);
		}

		virtual void Remove(const ValueType& value)
		{ TryRemove(value); }

		virtual bool TryRemove(const ValueType& value)
		{
			signal_locker l(_onChanged);
			if (!Wrapped_::TryRemove(value))
				return false;

			_onChanged(CollectionOp::Removed, value);
			return true;
		}

		virtual size_t RemoveWhere(const function<bool (const ValueType&)>& pred)
		{
			signal_locker l(_onChanged);
			size_t ret = 0;
			FOR_EACH(ValueType v IN Wrapped_::GetEnumerator() WHERE pred(v))
			{
				Wrapped_::Remove(v);
				_onChanged(CollectionOp::Removed, v);
				++ret;
			}
			return ret;
		}

		virtual void Clear()
		{
			signal_locker l(_onChanged);
			FOR_EACH(ValueType v IN Wrapped_::GetEnumerator())
			{
				Wrapped_::Remove(v);
				_onChanged(CollectionOp::Removed, v);
			}
		}

		virtual signal_connector<OnChangedSignature> OnChanged() const
		{ return _onChanged.connector(); }

		virtual const Mutex& GetSyncRoot() const
		{ return *_mutex; }

	private:
		void OnChangedPopulator(const function<OnChangedSignature>& slot)
		{
			FOR_EACH(ValueType v IN Wrapped_::GetEnumerator())
				slot(CollectionOp::Added, v);
		}
	};

	/** @} */

}

#endif
