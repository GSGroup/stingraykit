#ifndef STINGRAYKIT_COLLECTION_TRANSACTIONALSET_H
#define STINGRAYKIT_COLLECTION_TRANSACTIONALSET_H

#include <stingraykit/collection/EnumerableBuilder.h>

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/ITransactionalSet.h>

namespace stingray
{

	namespace Detail
	{
		template < typename T, typename Comparer >
		class TransactionalSetImpl
		{
		public:
			typedef T							ValueType;
			typedef std::set<T, Comparer>		Container;
			typedef DiffEntry<T>				DiffEntryType;
			typedef IEnumerable<DiffEntryType>	DiffType;
			STINGRAYKIT_DECLARE_PTR(DiffType);

		private:
			shared_ptr<Mutex>																	_mutex;
			Container																			_container;
			signal<void(const DiffTypePtr&), signal_policies::threading::ExternalMutexPointer>	_onChanged;
			u64																					_stamp;
			bool																				_transactionFlag;

		public:
			TransactionalSetImpl() : _mutex(new Mutex()),
				_onChanged(signal_policies::threading::ExternalMutexPointer(_mutex), bind(&TransactionalSetImpl::OnChangedPopulator, this, _1)),
				_stamp(0), _transactionFlag(false)
			{ }

			const Mutex& GetSync() const									{ return *_mutex; }
			Container& GetContainer()										{ return _container; }
			signal_connector<void(const DiffTypePtr&)> OnChanged() const	{ return _onChanged.connector(); }
			void InvokeOnChanged(const DiffTypePtr& diff)					{ _onChanged(diff); }
			u64& GetStamp()													{ return _stamp; }
			bool& GetTransactionFlag()										{ return _transactionFlag; }

		private:
			void OnChangedPopulator(const function<void(const DiffTypePtr&)>& slot) const
			{
				EnumerableBuilder<DiffEntryType> diff;
				for (typename Container::const_iterator it = _container.begin(); it != _container.end(); ++it)
					diff % DiffEntryType(*it, CollectionOp::Added);
				slot(diff.Get());
			}
		};


		template < typename T, typename Comparer >
		class SetTransactionImpl
		{
			typedef TransactionalSetImpl<T, Comparer>	SetImpl;
			STINGRAYKIT_DECLARE_PTR(SetImpl);

		public:
			typedef typename SetImpl::Container	Container;

		private:
			SetImplPtr	_setImpl;
			MutexLock	_mutexLock;
			Container	_added;
			Container	_removed;

		public:
			SetTransactionImpl(const SetImplPtr& setImpl) :
				_setImpl(setImpl), _mutexLock(setImpl->GetSync())
			{ }

			Container& GetAdded()		{ return _added; }
			Container& GetRemoved()		{ return _removed; }
			Container& GetContainer()	{ return _setImpl->GetContainer(); }
			u64& GetStamp()				{ return _setImpl->GetStamp(); }
		};


		template < typename T, typename Comparer >
		struct TransactionalSetEnumerator : public virtual IEnumerator<T>
		{
			typedef SetTransactionImpl<T, Comparer>		TransactionImpl;
			STINGRAYKIT_DECLARE_PTR(TransactionImpl);

			typedef typename TransactionImpl::Container	Container;

		private:
			TransactionImplPtr					_transactionImpl;
			u64									_stamp;
			typename Container::const_iterator	_containerIter;
			typename Container::const_iterator	_addedIter;
			typename Container::const_iterator	_removedIter;
			bool								_currentItemWasAdded;

		public:
			TransactionalSetEnumerator(const TransactionImplPtr& transactionImpl) :
				_transactionImpl(transactionImpl),
				_stamp(transactionImpl->GetStamp()),
				_containerIter(transactionImpl->GetContainer().begin()),
				_addedIter(transactionImpl->GetAdded().begin()),
				_removedIter(transactionImpl->GetRemoved().begin())
			{ _currentItemWasAdded = AddedValid() && (!ContainerValid() || Comparer()(*_addedIter, *_containerIter)); }

			virtual bool Valid() const	{ CheckStamp(); return ContainerValid() || AddedValid(); }
			virtual T Get() const		{ CheckStamp(); return _currentItemWasAdded ? *_addedIter : *_containerIter; }

			virtual void Next()
			{
				CheckStamp();

				if (_currentItemWasAdded)
					++_addedIter;
				else
				{
					NextContainerItem();
					while (RemovedValid() && !Comparer()(*_containerIter, *_removedIter)) // current item was removed
						NextContainerItem();
				}

				_currentItemWasAdded = AddedValid() && (!ContainerValid() || Comparer()(*_addedIter, *_containerIter));
			}

		private:
			void CheckStamp() const		{ STINGRAYKIT_CHECK(_stamp == _transactionImpl->GetStamp(), "Container was modified during enumeration!"); }
			bool AddedValid() const		{ return _addedIter != _transactionImpl->GetAdded().end(); }
			bool RemovedValid() const	{ return _removedIter != _transactionImpl->GetRemoved().end(); }
			bool ContainerValid() const	{ return _containerIter != _transactionImpl->GetContainer().end(); }

			void NextContainerItem()
			{
				++_containerIter;
				if (RemovedValid() && Comparer()(*_removedIter, *_containerIter)) // container can't reach end until removed do
					++_removedIter;
			}
		};


		template < typename T, typename Comparer >
		class SetTransaction : public virtual ISetTransaction<T>
		{
			typedef ISetTransaction<T>					base;

			typedef TransactionalSetImpl<T, Comparer>	SetImpl;
			STINGRAYKIT_DECLARE_PTR(SetImpl);

			typedef SetTransactionImpl<T, Comparer>		TransactionImpl;
			STINGRAYKIT_DECLARE_PTR(TransactionImpl);

			typedef typename SetImpl::Container			Container;

		private:
			SetImplPtr			_setImpl;
			TransactionImplPtr	_transactionImpl;
			u64					_stamp;

		public:
			SetTransaction(const SetImplPtr& setImpl) :
				_setImpl(setImpl),
				_transactionImpl(make_shared<TransactionImpl>(setImpl)),
				_stamp(setImpl->GetStamp())
			{ STINGRAYKIT_CHECK(!_setImpl->GetTransactionFlag(), "Another transaction exist!"); _setImpl->GetTransactionFlag() = true; }

			virtual ~SetTransaction()
			{ _setImpl->GetTransactionFlag() = false; }

			virtual int GetCount() const
			{ return GetContainer().size() + GetAdded().size() - GetRemoved().size(); }

			virtual bool IsEmpty() const
			{ return GetAdded().empty() && GetContainer().size() == GetRemoved().size(); }

			virtual bool Contains(const T& value) const
			{ return GetAdded().count(value) != 0 || (GetRemoved().count(value) == 0 && GetContainer().count(value) != 0); }

			virtual void Add(const T& value)
			{
				typename Container::const_iterator it = GetContainer().find(value);
				if (it != GetContainer().end())
					GetRemoved().insert(*it);

				it = GetAdded().find(value);
				if (it != GetAdded().end())
					GetAdded().erase(it);
				STINGRAYKIT_CHECK(GetAdded().insert(value).second, LogicException("Adding element that already exists!"));
			}

			virtual void Remove(const T& value)
			{
				typename Container::const_iterator it = GetAdded().find(value);
				if (it != GetAdded().end())
				{
					GetAdded().erase(it);
					return;
				}
				it = GetContainer().find(value);
				STINGRAYKIT_CHECK(it != GetContainer().end(), "No such value!");
				STINGRAYKIT_CHECK(GetRemoved().count(value) == 0, "No such value!");
				GetRemoved().insert(*it);
			}

			virtual void Clear()
			{
				GetAdded().clear();
				GetRemoved() = GetContainer();
			}

			virtual bool TryRemove(const T& value)
			{
				typename Container::const_iterator it = GetAdded().find(value);
				if (it != GetAdded().end())
				{
					GetAdded().erase(it);
					return true;
				}
				it = GetContainer().find(value);
				if (it == GetContainer().end() || GetRemoved().count(value) != 0)
					return false;
				GetRemoved().insert(*it);
				return true;
			}

			virtual typename base::DiffTypePtr Diff() const
			{
				typedef typename base::DiffEntryType DiffEntryType;
				EnumerableBuilder<DiffEntryType> diff;
				for (typename Container::const_iterator it = GetRemoved().begin(); it != GetRemoved().end(); ++it)
					diff % DiffEntryType(*it, CollectionOp::Removed);
				for (typename Container::const_iterator it = GetAdded().begin(); it != GetAdded().end(); ++it)
					diff % DiffEntryType(*it, CollectionOp::Added);
				return diff.Get();
			}

			virtual void Commit()
			{
				for (typename Container::const_iterator it = GetRemoved().begin(); it != GetRemoved().end(); ++it)
					GetContainer().erase(*it);
				for (typename Container::const_iterator it = GetAdded().begin(); it != GetAdded().end(); ++it)
					STINGRAYKIT_CHECK(GetContainer().insert(*it).second, LogicException("Adding element that already exists!"));
				_setImpl->InvokeOnChanged(Diff());
				_setImpl->GetStamp()++;
			}

			virtual void Revert()
			{
				GetAdded().clear();
				GetRemoved().clear();
			}

			virtual shared_ptr<IEnumerator<T> > GetEnumerator() const
			{ return make_shared<TransactionalSetEnumerator<T, Comparer> >(_transactionImpl); }

			virtual shared_ptr<IEnumerable<T> > Reverse() const
			{ STINGRAYKIT_THROW(NotImplementedException()); }

		private:
			Container& GetAdded()					{ return _transactionImpl->GetAdded(); }
			const Container& GetAdded() const		{ return _transactionImpl->GetAdded(); }

			Container& GetRemoved()					{ return _transactionImpl->GetRemoved(); }
			const Container& GetRemoved() const		{ return _transactionImpl->GetRemoved(); }

			Container& GetContainer()				{ return _transactionImpl->GetContainer(); }
			const Container& GetContainer() const	{ return _transactionImpl->GetContainer(); }
		};


		template < typename T, typename Comparer >
		class TransactionToken
		{
			typedef TransactionalSetImpl<T, Comparer>	SetImpl;
			STINGRAYKIT_DECLARE_PTR(SetImpl);

		private:
			SetImplPtr	_setImpl;

		public:
			TransactionToken(const SetImplPtr& setImpl) : _setImpl(setImpl)
			{ STINGRAYKIT_CHECK(!_setImpl->GetTransactionFlag(), "Another transaction exist!"); _setImpl->GetTransactionFlag() = true; }

			~TransactionToken()
			{ _setImpl->GetTransactionFlag() = false; }
		};
	}


	template < typename T, typename Comparer = comparers::Less >
	class TransactionalSet : public virtual ITransactionalSet<T>
	{
	public:
		typedef ITransactionalSet<T>			base;
		typedef typename base::DiffEntryType	DiffEntryType;
		typedef typename base::DiffTypePtr		DiffTypePtr;

	private:
		typedef Detail::TransactionalSetImpl<T, Comparer>	SetImpl;
		STINGRAYKIT_DECLARE_PTR(SetImpl);

		typedef typename SetImpl::Container					Container;

		typedef Detail::TransactionToken<T, Comparer>		TransactionToken;

	private:
		SetImplPtr	_setImpl;

	public:
		TransactionalSet() : _setImpl(make_shared<SetImpl>())
		{ }

		virtual const Mutex& GetSync() const									{ return _setImpl->GetSync(); }
		virtual signal_connector<void(const DiffTypePtr&)> OnChanged() const	{ return _setImpl->OnChanged(); }

		virtual int GetCount() const											{ MutexLock l(GetSync()); return GetContainer().size(); }
		virtual bool IsEmpty() const											{ MutexLock l(GetSync()); return GetContainer().empty(); }

		virtual bool Contains(const T& value) const								{ MutexLock l(GetSync()); return GetContainer().find(value) != GetContainer().end(); }

		virtual void Add(const T& value)
		{
			MutexLock l(GetSync());
			TransactionToken token(_setImpl);

			EnumerableBuilder<DiffEntryType> diff;
			typename Container::iterator it = GetContainer().find(value);
			if (it != GetContainer().end())
			{
				diff % DiffEntryType(*it, CollectionOp::Removed);
				GetContainer().erase(it);
			}
			diff % DiffEntryType(value, CollectionOp::Added);
			STINGRAYKIT_CHECK(GetContainer().insert(value).second, LogicException("Adding element that already exists!"));
			_setImpl->InvokeOnChanged(diff.Get());
			_setImpl->GetStamp()++;
		}

		virtual void Remove(const T& value)
		{
			MutexLock l(GetSync());
			TransactionToken token(_setImpl);

			EnumerableBuilder<DiffEntryType> diff;
			typename Container::iterator it = GetContainer().find(value);
			STINGRAYKIT_CHECK(it != GetContainer().end(), "Removing non-existing element!");
			diff % DiffEntryType(*it, CollectionOp::Removed);
			GetContainer().erase(it);
			_setImpl->InvokeOnChanged(diff.Get());
			_setImpl->GetStamp()++;
		}

		virtual bool TryRemove(const T& value)
		{
			MutexLock l(GetSync());
			TransactionToken token(_setImpl);

			EnumerableBuilder<DiffEntryType> diff;
			typename Container::iterator it = GetContainer().find(value);
			if (it == GetContainer().end())
				return false;
			diff % DiffEntryType(*it, CollectionOp::Removed);
			GetContainer().erase(it);
			_setImpl->InvokeOnChanged(diff.Get());
			_setImpl->GetStamp()++;
			return true;
		}

		virtual void Clear()
		{
			MutexLock l(GetSync());
			TransactionToken token(_setImpl);

			EnumerableBuilder<DiffEntryType> diff;
			for (typename Container::const_iterator it = GetContainer().begin(); it != GetContainer().end(); ++it)
				diff % DiffEntryType(*it, CollectionOp::Removed);
			GetContainer().clear();
			_setImpl->InvokeOnChanged(diff.Get());
			_setImpl->GetStamp()++;
		}

		virtual shared_ptr<IEnumerator<T> > GetEnumerator() const
		{ return make_shared<Detail::TransactionalSetEnumerator<T, Comparer> >(make_shared<Detail::SetTransactionImpl<T, Comparer> >(_setImpl)); }

		virtual shared_ptr<IEnumerable<T> > Reverse() const
		{ STINGRAYKIT_THROW(NotImplementedException()); }

		virtual typename base::TransactionPtr StartTransaction()
		{ return make_shared<Detail::SetTransaction<T, Comparer> >(_setImpl); }

	private:
		Container& GetContainer()				{ return _setImpl->GetContainer(); }
		const Container& GetContainer() const	{ return _setImpl->GetContainer(); }
	};
}

#endif