#ifndef STINGRAYKIT_COLLECTION_IOBSERVABLELIST_H
#define STINGRAYKIT_COLLECTION_IOBSERVABLELIST_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/ForEach.h>
#include <stingraykit/collection/IList.h>
#include <stingraykit/collection/ObservableCollectionLocker.h>
#include <stingraykit/signal/signals.h>
#include <stingraykit/toolkit.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename ValueType_ >
	struct IReadonlyObservableList : public virtual IReadonlyList<ValueType_>
	{
		typedef void OnChangedSignature(CollectionOp, size_t, const ValueType_&);

		virtual signal_connector<OnChangedSignature> OnChanged() const = 0;

		ObservableCollectionLockerPtr Lock() const { return make_shared<ObservableCollectionLocker>(*this); }

		virtual const Mutex& GetSyncRoot() const = 0;
	};


	template < typename ValueType_ >
	struct IObservableList : public virtual IList<ValueType_>, public virtual IReadonlyObservableList<ValueType_>
	{
	};


	template < typename Wrapped_ >
	struct ObservableListWrapper
		:	public Wrapped_,
			public virtual IObservableList<typename Wrapped_::ValueType>
	{
	public:
		typedef typename Wrapped_::ValueType						ValueType;
		typedef IObservableList<ValueType>							ObservableInterface;
		typedef typename ObservableInterface::OnChangedSignature	OnChangedSignature;

	private:
		shared_ptr<Mutex>																_mutex;
		signal<OnChangedSignature, signal_policies::threading::ExternalMutexPointer>	_onChanged;

	public:
		ObservableListWrapper()
			: _mutex(new Mutex()), _onChanged(signal_policies::threading::ExternalMutexPointer(_mutex), bind(&ObservableListWrapper::OnChangedPopulator, this, _1))
		{ }

		virtual signal_connector<OnChangedSignature> OnChanged() const
		{ return _onChanged.connector(); }

		virtual const Mutex& GetSyncRoot() const { return *_mutex; }

		virtual size_t GetCount() const
		{
			signal_locker l(_onChanged);
			return Wrapped_::GetCount();
		}

		virtual void Add(const ValueType& value)
		{
			signal_locker l(_onChanged);
			Wrapped_::Add(value);
			_onChanged(CollectionOp::Added, Wrapped_::GetCount() - 1, value);
		}

		virtual ValueType Get(size_t index) const
		{
			signal_locker l(_onChanged);
			return Wrapped_::Get(index);
		}

		virtual void Set(size_t index, const ValueType& value)
		{
			signal_locker l(_onChanged);
			Wrapped_::Set(index, value);
			_onChanged(CollectionOp::Updated, index, value);
		}

		virtual optional<size_t> IndexOf(const ValueType& obj) const
		{
			signal_locker l(_onChanged);
			return Wrapped_::IndexOf(obj);
		}

		virtual void Insert(size_t index, const ValueType& value)
		{
			signal_locker l(_onChanged);
			Wrapped_::Insert(index, value);
			_onChanged(CollectionOp::Added, index, value);
		}

		virtual void RemoveAt(size_t index)
		{
			signal_locker l(_onChanged);
			ValueType value = Get(index);
			Wrapped_::RemoveAt(index);
			_onChanged(CollectionOp::Removed, index, value);
		}

		virtual void Remove(const ValueType& value)
		{
			signal_locker l(_onChanged);
			if (const optional<size_t> index = Wrapped_::IndexOf(value))
				RemoveAt(*index);
		}

		virtual bool Contains(const ValueType& value) const
		{
			signal_locker l(_onChanged);
			return Wrapped_::Contains(value);
		}

		virtual bool TryGet(size_t index, ValueType& value) const
		{
			signal_locker l(_onChanged);
			return Wrapped_::TryGet(index, value);
		}

		virtual void Clear()
		{
			signal_locker l(_onChanged);
			while (!this->IsEmpty())
				RemoveAt(0);
		}

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
