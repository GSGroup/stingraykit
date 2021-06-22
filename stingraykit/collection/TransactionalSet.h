#ifndef STINGRAYKIT_COLLECTION_TRANSACTIONALSET_H
#define STINGRAYKIT_COLLECTION_TRANSACTIONALSET_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableBuilder.h>
#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/ITransactionalSet.h>
#include <stingraykit/signal/signals.h>

#include <set>

namespace stingray
{

	namespace Detail
	{
		template < typename T, typename Comparer >
		class TransactionalSetImpl
		{
			typedef signal_policies::threading::ExternalMutexPointer ExternalMutexPointer;

		public:
			typedef T							ValueType;
			typedef std::set<T, Comparer>		Container;
			typedef DiffEntry<T>				DiffEntryType;
			typedef IEnumerable<DiffEntryType>	DiffType;
			STINGRAYKIT_DECLARE_PTR(DiffType);

		private:
			shared_ptr<Mutex>																_mutex;
			Mutex																			_transactionMutex;
			Container																		_container;
			signal<void(const DiffTypePtr&), ExternalMutexPointer>							_onChanged;
			u64																				_stamp;
			bool																			_transactionFlag;

		public:
			TransactionalSetImpl()
				:	_mutex(make_shared_ptr<Mutex>()),
					_onChanged(ExternalMutexPointer(_mutex), Bind(&TransactionalSetImpl::OnChangedPopulator, this, _1)),
					_stamp(0),
					_transactionFlag(false)
			{ }

			const Mutex& GetStateMutex() const								{ return *_mutex; }
			const Mutex& GetTransactionMutex() const						{ return _transactionMutex; }
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
					diff % DiffEntryType(CollectionOp::Added, *it);
				slot(diff.Get());
			}
		};


		template < typename T, typename Comparer >
		class SetTransactionImpl
		{
			typedef TransactionalSetImpl<T, Comparer> SetImpl;
			STINGRAYKIT_DECLARE_PTR(SetImpl);

		public:
			typedef typename SetImpl::Container       Container;

		private:
			SetImplPtr	_setImpl;
			MutexLock	_mutexLock;
			Container	_added;
			Container	_removed;
			u64			_stamp;

		public:
			SetTransactionImpl(const SetImplPtr& setImpl) :
				_setImpl(setImpl), _mutexLock(setImpl->GetTransactionMutex()), _stamp(0)
			{ }

			Container& GetAdded()		{ return _added; }
			Container& GetRemoved()		{ return _removed; }
			Container& GetContainer()	{ return _setImpl->GetContainer(); }
			u64& GetStamp()				{ return _stamp; }
		};


		template < typename T, typename Comparer >
		struct SetTransactionEnumerator : public virtual IEnumerator<T>
		{
			typedef SetTransactionImpl<T, Comparer>     TransactionImpl;
			STINGRAYKIT_DECLARE_PTR(TransactionImpl);

			typedef typename TransactionImpl::Container Container;

		private:
			TransactionImplPtr					_transactionImpl;
			u64									 _stamp;
			typename Container::const_iterator	_containerIter;
			typename Container::const_iterator	_addedIter;
			typename Container::const_iterator	_removedIter;
			bool								_currentItemWasAdded;

		public:
			SetTransactionEnumerator(const TransactionImplPtr& transactionImpl) :
				_transactionImpl(transactionImpl),
				_stamp(transactionImpl->GetStamp()),
				_containerIter(transactionImpl->GetContainer().begin()),
				_addedIter(transactionImpl->GetAdded().begin()),
				_removedIter(transactionImpl->GetRemoved().begin())
			{ _currentItemWasAdded = AddedValid() && (!ContainerValid() || Comparer()(*_addedIter, *_containerIter)); }

			bool Valid() const override	{ CheckStamp(); return ContainerValid() || AddedValid(); }
			T Get() const override		{ CheckStamp(); return _currentItemWasAdded ? *_addedIter : *_containerIter; }

			void Next() override
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
			typedef DiffEntry<T>						DiffEntryType;

			typedef TransactionalSetImpl<T, Comparer>	SetImpl;
			STINGRAYKIT_DECLARE_PTR(SetImpl);

			typedef SetTransactionImpl<T, Comparer>		TransactionImpl;
			STINGRAYKIT_DECLARE_PTR(TransactionImpl);

			typedef typename SetImpl::Container			Container;

		private:
			SetImplPtr				_setImpl;
			TransactionImplPtr		_transactionImpl;

		public:
			SetTransaction(const SetImplPtr& setImpl) :
				_setImpl(setImpl),
				_transactionImpl(make_shared_ptr<TransactionImpl>(setImpl))
			{ STINGRAYKIT_CHECK(!_setImpl->GetTransactionFlag(), "Another transaction exist!"); _setImpl->GetTransactionFlag() = true; }

			~SetTransaction() override
			{ _setImpl->GetTransactionFlag() = false; }

			shared_ptr<IEnumerator<T> > GetEnumerator() const override
			{ return make_shared_ptr<SetTransactionEnumerator<T, Comparer> >(_transactionImpl); }

			shared_ptr<IEnumerable<T> > Reverse() const override
			{ STINGRAYKIT_THROW(NotImplementedException()); }

			shared_ptr<IEnumerator<T> > Find(const T& value) const override
			{ STINGRAYKIT_THROW(NotImplementedException()); }

			shared_ptr<IEnumerator<T> > ReverseFind(const T& value) const override
			{ STINGRAYKIT_THROW(NotImplementedException()); }

			size_t GetCount() const override
			{ return GetContainer().size() + GetAdded().size() - GetRemoved().size(); }

			bool IsEmpty() const override
			{ return GetAdded().empty() && GetContainer().size() == GetRemoved().size(); }

			bool Contains(const T& value) const override
			{ return GetAdded().count(value) != 0 || (GetRemoved().count(value) == 0 && GetContainer().count(value) != 0); }

			void Add(const T& value) override
			{
				_transactionImpl->GetStamp()++;

				typename Container::const_iterator it = GetRemoved().find(value);
				if (it != GetRemoved().end())
				{
					GetRemoved().erase(it);
					return;
				}
				STINGRAYKIT_CHECK(GetContainer().count(value) == 0, "Value already exists!");
				STINGRAYKIT_CHECK(GetAdded().insert(value).second, "Value already exists!");
			}

			void Remove(const T& value) override
			{
				_transactionImpl->GetStamp()++;

				typename Container::const_iterator it = GetAdded().find(value);
				if (it != GetAdded().end())
				{
					GetAdded().erase(it);
					return;
				}
				it = GetContainer().find(value);
				STINGRAYKIT_CHECK(it != GetContainer().end(), "No such value!");
				STINGRAYKIT_CHECK(GetRemoved().insert(*it).second, "No such value!");
			}

			bool TryRemove(const T& value) override
			{
				_transactionImpl->GetStamp()++;

				typename Container::const_iterator it = GetAdded().find(value);
				if (it != GetAdded().end())
				{
					GetAdded().erase(it);
					return true;
				}
				it = GetContainer().find(value);
				if (it == GetContainer().end())
					return false;
				return GetRemoved().insert(*it).second;
			}

			size_t RemoveWhere(const function<bool (const T&)>& pred) override
			{
				_transactionImpl->GetStamp()++;
				size_t ret = 0;

				for (typename Container::iterator it = GetAdded().begin(); it != GetAdded().end(); )
				{
					const typename Container::iterator cur = it++;
					if (!pred(*cur))
						continue;

					GetAdded().erase(cur);
					++ret;
				}

				for (typename Container::iterator it = GetContainer().begin(); it != GetContainer().end(); )
				{
					const typename Container::iterator cur = it++;
					if (!pred(*cur))
						continue;

					if (GetRemoved().insert(*cur).second)
						++ret;
				}

				return ret;
			}

			void Clear() override
			{
				_transactionImpl->GetStamp()++;

				GetAdded().clear();
				GetRemoved() = GetContainer();
			}

			void Apply(const DiffEntryType& entry) override
			{
				switch (entry.Op)
				{
				case CollectionOp::Added:		Add(entry.Item); break;
				case CollectionOp::Removed:		Remove(entry.Item); break;
				case CollectionOp::Updated:
					STINGRAYKIT_THROW(NotSupportedException());
				}
			}

			void Commit() override
			{
				if (GetRemoved().empty() && GetAdded().empty())
					return;

				{
					MutexLock l(_setImpl->GetStateMutex());
					for (typename Container::const_iterator it = GetRemoved().begin(); it != GetRemoved().end(); ++it)
						GetContainer().erase(*it);
					for (typename Container::const_iterator it = GetAdded().begin(); it != GetAdded().end(); ++it)
						STINGRAYKIT_CHECK(GetContainer().insert(*it).second, LogicException("Adding element that already exists!"));
				}

				_setImpl->InvokeOnChanged(Diff());
				_transactionImpl->GetStamp()++;

				GetAdded().clear();
				GetRemoved().clear();
			}

			void Revert() override
			{
				_transactionImpl->GetStamp()++;

				GetAdded().clear();
				GetRemoved().clear();
			}

			typename base::DiffTypePtr Diff() const override
			{
				typedef typename base::DiffEntryType DiffEntryType;
				EnumerableBuilder<DiffEntryType> diff;
				for (typename Container::const_iterator it = GetRemoved().begin(); it != GetRemoved().end(); ++it)
					diff % DiffEntryType(CollectionOp::Removed, *it);
				for (typename Container::const_iterator it = GetAdded().begin(); it != GetAdded().end(); ++it)
					diff % DiffEntryType(CollectionOp::Added, *it);
				return diff.Get();
			}

			bool IsDirty() const override
			{ return !GetAdded().empty() || !GetRemoved().empty(); }

		private:
			Container& GetAdded()					{ return _transactionImpl->GetAdded(); }
			const Container& GetAdded() const		{ return _transactionImpl->GetAdded(); }

			Container& GetRemoved()					{ return _transactionImpl->GetRemoved(); }
			const Container& GetRemoved() const		{ return _transactionImpl->GetRemoved(); }

			Container& GetContainer()				{ return _transactionImpl->GetContainer(); }
			const Container& GetContainer() const	{ return _transactionImpl->GetContainer(); }
		};


		template < typename T, typename Comparer >
		struct SetEnumerator : public virtual IEnumerator<T>
		{
			typedef TransactionalSetImpl<T, Comparer>	SetImpl;
			STINGRAYKIT_DECLARE_PTR(SetImpl);

			typedef typename SetImpl::Container			Container;

		private:
			SetImplPtr							_setImpl;
			MutexLock							_mutexLock;
			u64									_stamp;
			typename Container::const_iterator	_containerIter;

		public:
			SetEnumerator(const SetImplPtr& setImpl) :
				_setImpl(setImpl),
				_mutexLock(setImpl->GetStateMutex()),
				_stamp(setImpl->GetStamp()),
				_containerIter(setImpl->GetContainer().begin())
			{ }

			bool Valid() const override	{ CheckStamp(); return _containerIter != _setImpl->GetContainer().end(); }
			T Get() const override		{ CheckStamp(); return *_containerIter; }
			void Next() override		{ CheckStamp(); ++_containerIter; }

		private:
			void CheckStamp() const		{ STINGRAYKIT_CHECK(_stamp == _setImpl->GetStamp(), "Container was modified during enumeration!"); }
		};


		template < typename T, typename Comparer >
		class TransactionToken
		{
			typedef TransactionalSetImpl<T, Comparer>    SetImpl;
			STINGRAYKIT_DECLARE_PTR(SetImpl);

		private:
			SetImplPtr    _setImpl;

		public:
			TransactionToken(const SetImplPtr& setImpl) : _setImpl(setImpl)
			{ STINGRAYKIT_CHECK(!_setImpl->GetTransactionFlag(), "Another transaction exists!"); _setImpl->GetTransactionFlag() = true; }

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
		SetImplPtr    _setImpl;

	public:
		TransactionalSet() : _setImpl(make_shared_ptr<SetImpl>())
		{ }

		shared_ptr<IEnumerator<T> > GetEnumerator() const override
		{ return make_shared_ptr<Detail::SetEnumerator<T, Comparer> >(_setImpl); }

		shared_ptr<IEnumerable<T> > Reverse() const override
		{ STINGRAYKIT_THROW(NotImplementedException()); }

		shared_ptr<IEnumerator<T> > Find(const T& value) const override
		{ STINGRAYKIT_THROW(NotImplementedException()); }

		shared_ptr<IEnumerator<T> > ReverseFind(const T& value) const override
		{ STINGRAYKIT_THROW(NotImplementedException()); }

		size_t GetCount() const override
		{ MutexLock l(GetSyncRoot()); return GetContainer().size(); }

		bool IsEmpty() const override
		{ MutexLock l(GetSyncRoot()); return GetContainer().empty(); }

		bool Contains(const T& value) const override
		{ MutexLock l(GetSyncRoot()); return GetContainer().find(value) != GetContainer().end(); }

		typename base::TransactionTypePtr StartTransaction(const ICancellationToken& token = DummyCancellationToken()) override
		{ return make_shared_ptr<Detail::SetTransaction<T, Comparer> >(_setImpl); }

		signal_connector<void(const DiffTypePtr&)> OnChanged() const override
		{ return _setImpl->OnChanged(); }

		const Mutex& GetSyncRoot() const override
		{ return _setImpl->GetStateMutex(); }

	private:
		Container& GetContainer()				{ return _setImpl->GetContainer(); }
		const Container& GetContainer() const	{ return _setImpl->GetContainer(); }
	};
}

#endif
