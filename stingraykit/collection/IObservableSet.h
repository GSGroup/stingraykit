#ifndef STINGRAYKIT_COLLECTION_IOBSERVABLESET_H
#define STINGRAYKIT_COLLECTION_IOBSERVABLESET_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/collection/ForEach.h>
#include <stingraykit/collection/ISet.h>
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
	struct IReadonlyObservableSet : public virtual IReadonlySet<ValueType_>
	{
		virtual signal_connector<void(CollectionOp, const ValueType_&)>	OnChanged() const = 0;

		ObservableCollectionLockerPtr Lock() const { return make_shared<ObservableCollectionLocker>(*this); }

		virtual const Mutex& GetSyncRoot() const = 0;

	protected:
		virtual void InvokeOnChanged(CollectionOp, const ValueType_&) = 0;
	};


	template < typename ValueType_ >
	struct IObservableSet : public virtual ISet<ValueType_>, public virtual IReadonlyObservableSet<ValueType_>
	{
	};


	template < typename Wrapped_ >
	struct ObservableSetWrapper
		:	public Wrapped_,
			public virtual IObservableSet<typename Wrapped_::ValueType>
	{
		typedef typename Wrapped_::ValueType					ValueType;

		virtual int GetCount() const
		{
			MutexLock l(this->GetSyncRoot());
			return Wrapped_::GetCount();
		}

		virtual void Add(const ValueType& value)
		{
			MutexLock l(this->GetSyncRoot());
			bool signal = !Wrapped_::Contains(value);
			Wrapped_::Add(value);
			if (signal)
				this->InvokeOnChanged(CollectionOp::Added, value);
		}

		virtual void Remove(const ValueType& value)
		{
			MutexLock l(this->GetSyncRoot());
			bool signal = Wrapped_::Contains(value);
			Wrapped_::Remove(value);
			if (signal)
				this->InvokeOnChanged(CollectionOp::Removed, value);
		}

		virtual bool TryRemove(const ValueType& value)
		{
			MutexLock l(this->GetSyncRoot());
			if (!Wrapped_::TryRemove(value))
				return false;

			this->InvokeOnChanged(CollectionOp::Removed, value);
			return true;
		}

		virtual bool Contains(const ValueType& value) const
		{
			MutexLock l(this->GetSyncRoot());
			return Wrapped_::Contains(value);
		}

		virtual void Clear()
		{
			MutexLock l(this->GetSyncRoot());
			FOR_EACH(ValueType v IN this->GetEnumerator())
				this->InvokeOnChanged(CollectionOp::Removed, v);
			Wrapped_::Clear();
		}

		virtual shared_ptr<IEnumerator<ValueType> > GetEnumerator() const
		{
			MutexLock l(this->GetSyncRoot());
			return Wrapped_::GetEnumerator();
		}

		virtual shared_ptr<IEnumerable<ValueType> > Reverse() const
		{
			MutexLock l(this->GetSyncRoot());
			return Wrapped_::Reverse();
		}
	};

	/** @} */

}


#endif
