#ifndef STINGRAYKIT_COLLECTION_IOBSERVABLESET_H
#define STINGRAYKIT_COLLECTION_IOBSERVABLESET_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/ForEach.h>
#include <stingraykit/collection/ISet.h>
#include <stingraykit/collection/ObservableCollectionLocker.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename ValueType_ >
	struct IReadonlyObservableSet : public virtual IReadonlySet<ValueType_>
	{
		typedef void OnChangedSignature(CollectionOp, const ValueType_&);

		virtual signal_connector<OnChangedSignature> OnChanged() const = 0;

		ObservableCollectionLockerPtr Lock() const { return make_shared<ObservableCollectionLocker>(*this); }

		virtual const Mutex& GetSyncRoot() const = 0;
	};


	template < typename ValueType_ >
	struct IObservableSet : public virtual ISet<ValueType_>, public virtual IReadonlyObservableSet<ValueType_>
	{
	};


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
			: _mutex(new Mutex()), _onChanged(ExternalMutexPointer(_mutex), bind(&ObservableSetWrapper::OnChangedPopulator, this, _1))
		{ }

		virtual const Mutex& GetSyncRoot() const { return *_mutex; }

		virtual signal_connector<OnChangedSignature> OnChanged() const
		{ return _onChanged.connector(); }

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

		virtual void Add(const ValueType& value)
		{
			signal_locker l(_onChanged);
			bool signal = !Wrapped_::Contains(value);
			Wrapped_::Add(value);
			if (signal)
				_onChanged(CollectionOp::Added, value);
		}

		virtual void Remove(const ValueType& value)
		{
			signal_locker l(_onChanged);
			bool signal = Wrapped_::Contains(value);
			Wrapped_::Remove(value);
			if (signal)
				_onChanged(CollectionOp::Removed, value);
		}

		virtual bool TryRemove(const ValueType& value)
		{
			signal_locker l(_onChanged);
			if (!Wrapped_::TryRemove(value))
				return false;

			_onChanged(CollectionOp::Removed, value);
			return true;
		}

		virtual bool Contains(const ValueType& value) const
		{
			signal_locker l(_onChanged);
			return Wrapped_::Contains(value);
		}

		virtual void Clear()
		{
			signal_locker l(_onChanged);
			FOR_EACH(ValueType v IN this->GetEnumerator())
				_onChanged(CollectionOp::Removed, v);
			Wrapped_::Clear();
		}

		virtual size_t RemoveWhere(const function<bool (const ValueType&)>& pred)
		{
			signal_locker l(_onChanged);
			bool ret = 0;
			FOR_EACH(ValueType v IN this->GetEnumerator() WHERE pred(v))
			{
				Wrapped_::Remove(v);
				_onChanged(CollectionOp::Removed, v);
				++ret;
			}
			return ret;
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
		void OnChangedPopulator(const function<OnChangedSignature>& slot)
		{
			FOR_EACH(ValueType v IN this->GetEnumerator())
				slot(CollectionOp::Added, v);
		}
	};

	/** @} */

}

#endif
