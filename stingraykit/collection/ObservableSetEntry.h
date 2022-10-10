#ifndef STINGRAYKIT_COLLECTION_OBSERVABLESETENTRY_H
#define STINGRAYKIT_COLLECTION_OBSERVABLESETENTRY_H

// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/IObservableSet.h>
#include <stingraykit/signal/signals.h>
#include <stingraykit/IObservableValue.h>

namespace stingray
{

	template < typename T, typename EqualsCmp_ >
	class ReadonlyObservableSetEntry : public virtual IReadonlyObservableValue<bool>
	{
		typedef ReadonlyObservableSetEntry Self;

		typedef IReadonlyObservableSet<T> SetType;
		STINGRAYKIT_DECLARE_PTR(SetType);

		typedef signal_policies::threading::ExternalMutexPointer ExternalMutexPointer;

	private:
		SetTypePtr											_set;
		T													_key;

		signal<OnChangedSignature, ExternalMutexPointer>	_onChanged;
		Token												_connection;

	public:
		ReadonlyObservableSetEntry(const SetTypePtr& set, const T& key)
			:	_set(set),
				_key(key),
				_onChanged(ExternalMutexPointer(shared_ptr<const Mutex>(_set, &_set->GetSyncRoot())), Bind(&Self::OnChangedPopulator, this, _1)),
				_connection(_set->OnChanged().connect(Bind(&Self::InvokeOnChanged, this, _1, _2), false))
		{ }

		virtual bool Get() const
		{ return _set->Contains(_key); }

		virtual signal_connector<OnChangedSignature> OnChanged() const
		{ return _onChanged.connector(); }

		virtual const Mutex& GetSyncRoot() const
		{ return _set->GetSyncRoot(); }

	private:
		void InvokeOnChanged(CollectionOp op, const T& key)
		{
			if (EqualsCmp_()(_key, key))
				_onChanged(op == CollectionOp::Added);
		}

		void OnChangedPopulator(const function<OnChangedSignature>& slot) const
		{ slot(Get()); }
	};


	template < typename T, typename EqualsCmp_ >
	class ObservableSetEntry : public virtual IObservableValue<bool>, public ReadonlyObservableSetEntry<T, EqualsCmp_>
	{
		typedef IObservableSet<T> SetType;
		STINGRAYKIT_DECLARE_PTR(SetType);

	private:
		SetTypePtr		_set;
		T				_key;

	public:
		ObservableSetEntry(const SetTypePtr& set, const T& key)
			:	ReadonlyObservableSetEntry<T, EqualsCmp_>(set, key),
				_set(set),
				_key(key)
		{ }

		virtual void Set(bool value)
		{
			if (value)
				_set->Add(_key);
			else
				_set->TryRemove(_key);
		}
	};


	template < typename T, typename EqualsCmp >
	shared_ptr<ReadonlyObservableSetEntry<T, EqualsCmp> > GetSetEntry(const shared_ptr<IReadonlyObservableSet<T> >& set, const T& key, const EqualsCmp& equalsCmp)
	{ return make_shared_ptr<ReadonlyObservableSetEntry<T, EqualsCmp> >(set, key); }


	template < typename T >
	shared_ptr<ReadonlyObservableSetEntry<T, comparers::Equals> > GetSetEntry(const shared_ptr<IReadonlyObservableSet<T> >& set, const T& key)
	{ return make_shared_ptr<ReadonlyObservableSetEntry<T, comparers::Equals> >(set, key); }


	template < typename T, typename EqualsCmp >
	shared_ptr<ObservableSetEntry<T, EqualsCmp> > GetSetEntry(const shared_ptr<IObservableSet<T> >& set, const T& key, const EqualsCmp& equalsCmp)
	{ return make_shared_ptr<ObservableSetEntry<T, EqualsCmp> >(set, key); }


	template < typename T >
	shared_ptr<ObservableSetEntry<T, comparers::Equals> > GetSetEntry(const shared_ptr<IObservableSet<T> >& set, const T& key)
	{ return make_shared_ptr<ObservableSetEntry<T, comparers::Equals> >(set, key); }

}

#endif
