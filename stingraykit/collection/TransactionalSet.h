#ifndef STINGRAYKIT_COLLECTION_TRANSACTIONALSET_H
#define STINGRAYKIT_COLLECTION_TRANSACTIONALSET_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/ITransactionalSet.h>
#include <stingraykit/collection/TransactionHelpers.h>
#include <stingraykit/diagnostics/ExecutorsProfiler.h>
#include <stingraykit/signal/signals.h>

#include <set>

namespace stingray
{

	template < typename ValueType_, typename ValueLessComparer_ = comparers::Less >
	class TransactionalSet : public virtual ITransactionalSet<ValueType_>
	{
		STINGRAYKIT_NONCOPYABLE(TransactionalSet);

	private:
		using EMP = signal_policies::threading::ExternalMutexPointer;

	public:
		using base = ITransactionalSet<ValueType_>;

		using ValueType = typename base::ValueType;

		using DiffEntryType = typename base::DiffEntryType;
		using DiffTypePtr = typename base::DiffTypePtr;
		using TransactionTypePtr = typename base::TransactionTypePtr;

		using ValueLessComparer = ValueLessComparer_;

	private:
		using SetType = std::set<ValueType, ValueLessComparer>;
		STINGRAYKIT_DECLARE_PTR(SetType);
		STINGRAYKIT_DECLARE_CONST_PTR(SetType);

		struct Holder
		{
			const SetTypeConstPtr		Items;

			Holder(const SetTypeConstPtr& items)
				:	Items(STINGRAYKIT_REQUIRE_NOT_NULL(items))
			{ }
		};
		STINGRAYKIT_DECLARE_PTR(Holder);

		struct Utils
		{
			static HolderPtr GetItemsHolder(const SetTypeConstPtr& items, HolderWeakPtr& itemsHolder_)
			{
				HolderPtr itemsHolder = itemsHolder_.lock();

				if (!itemsHolder)
					itemsHolder_ = (itemsHolder = make_shared_ptr<Holder>(items));

				return itemsHolder;
			}

			static bool CopyOnWrite(SetTypePtr& items, HolderWeakPtr& itemsHolder, const SetType* src)
			{
				if (itemsHolder.expired())
					return false;

				items = src ? make_shared_ptr<SetType>(*src) : make_shared_ptr<SetType>();
				itemsHolder.reset();
				return true;
			}

			static std::string GetProfilerMessage()
			{ return StringBuilder() % "Starting transaction on " % TypeInfo(typeid(TransactionalSet)).GetClassName(); }
		};

		class ReverseEnumerable : public virtual IEnumerable<ValueType>
		{
		private:
			const HolderPtr				_holder;

		public:
			explicit ReverseEnumerable(const HolderPtr& holder)
				:	_holder(STINGRAYKIT_REQUIRE_NOT_NULL(holder))
			{ }

			shared_ptr<IEnumerator<ValueType>> GetEnumerator() const override
			{ return EnumeratorFromStlIterators(_holder->Items->rbegin(), _holder->Items->rend(), _holder); }
		};

		struct ForwardEnumerationDirection
		{
			using ItemType = ValueType;

			using CollectionType = Holder;
			using IteratorType = typename SetType::const_iterator;
			using LessComparer = ValueLessComparer;

			static IteratorType Begin(const CollectionType& holder)
			{ return holder.Items->begin(); }

			static IteratorType End(const CollectionType& holder)
			{ return holder.Items->end(); }

			static ItemType ToItem(const ValueType& value)
			{ return value; }
		};

		struct ForwardFindEnumerationDirection : public ForwardEnumerationDirection
		{
			using base = ForwardEnumerationDirection;

			const ValueType			Value;

			explicit ForwardFindEnumerationDirection(const ValueType& value) : Value(value) { }

			typename base::IteratorType Begin(const typename base::CollectionType& holder)
			{ return holder.Items->lower_bound(Value); }
		};

		struct ReverseEnumerationDirection
		{
			using ItemType = ValueType;

			using CollectionType = Holder;
			using IteratorType = typename SetType::const_reverse_iterator;

			struct LessComparer : public function_info<bool, UnspecifiedParamTypes>
			{
				bool operator () (const ValueType& lhs, const ValueType& rhs) const
				{ return ValueLessComparer()(rhs, lhs); }
			};

			static IteratorType Begin(const CollectionType& holder)
			{ return holder.Items->rbegin(); }

			static IteratorType End(const CollectionType& holder)
			{ return holder.Items->rend(); }

			static ItemType ToItem(const ValueType& value)
			{ return value; }
		};

		struct ReverseFindEnumerationDirection : public ReverseEnumerationDirection
		{
			using base = ReverseEnumerationDirection;
			using IteratorType = typename base::IteratorType;

			const ValueType			Value;

			explicit ReverseFindEnumerationDirection(const ValueType& value) : Value(value) { }

			IteratorType Begin(const typename base::CollectionType& holder)
			{
				typename SetType::const_iterator itemsIt = holder.Items->lower_bound(Value);

				if (itemsIt == holder.Items->end() || ValueLessComparer()(Value, *itemsIt))
					return IteratorType(itemsIt);

				return IteratorType(++itemsIt);
			}
		};

		class ImplData
		{
		public:
			const shared_ptr<Mutex>						Guard;
			SetTypePtr									Items;
			mutable HolderWeakPtr						ItemsHolder;

			bool										HasTransaction;
			ConditionVariable							TransactionCompleted;
			signal<void (const DiffTypePtr&), EMP>		OnChanged;

		public:
			ImplData()
				:	Guard(make_shared_ptr<Mutex>()),
					Items(make_shared_ptr<SetType>()),
					HasTransaction(false),
					OnChanged(EMP(Guard), Bind(&ImplData::OnChangedPopulator, this, _1))
			{ }

			HolderPtr GetItemsHolder() const
			{
				MutexLock l(*Guard);
				return Utils::GetItemsHolder(Items, ItemsHolder);
			}

		private:
			void OnChangedPopulator(const function<void (const DiffTypePtr&)>& slot) const
			{ slot(WrapEnumerable(EnumerableFromStlContainer(*Items, GetItemsHolder()), Bind(&MakeDiffEntry<ValueType>, CollectionOp::Added, _1))); }
		};
		STINGRAYKIT_DECLARE_PTR(ImplData);

		class Transaction : public virtual ISetTransaction<ValueType>
		{
			STINGRAYKIT_NONCOPYABLE(Transaction);

		private:
			ImplDataPtr						_impl;
			SetTypePtr						_added;
			mutable HolderWeakPtr			_addedHolder;
			SetTypePtr						_removed;
			mutable HolderWeakPtr			_removedHolder;

		public:
			Transaction(const ImplDataPtr& impl, const ICancellationToken& token)
				:	_impl(STINGRAYKIT_REQUIRE_NOT_NULL(impl)),
					_added(make_shared_ptr<SetType>()),
					_removed(make_shared_ptr<SetType>())
			{
				optional<AsyncProfiler::Session> profilerSession;

				MutexLock l(*_impl->Guard);

				while (_impl->HasTransaction)
				{
					if (!profilerSession)
						profilerSession.emplace(ExecutorsProfiler::Instance().GetProfiler(), &Utils::GetProfilerMessage, TimeDuration::Second(), AsyncProfiler::NameGetterTag());

					switch (_impl->TransactionCompleted.Wait(*_impl->Guard, token))
					{
					case ConditionWaitResult::Broadcasted:	continue;
					case ConditionWaitResult::Cancelled:	STINGRAYKIT_THROW(OperationCancelledException());
					case ConditionWaitResult::TimedOut:		STINGRAYKIT_THROW(TimeoutException());
					}
				}

				_impl->HasTransaction = true;
			}

			~Transaction() override
			{
				MutexLock l(*_impl->Guard);
				_impl->HasTransaction = false;
				STINGRAYKIT_TRY_NO_MESSAGE(_impl->TransactionCompleted.Broadcast());
			}

			shared_ptr<IEnumerator<ValueType>> GetEnumerator() const override
			{ return make_shared_ptr<SortedCollectionTransactionEnumerator<ForwardEnumerationDirection>>(_impl->GetItemsHolder(), GetAddedHolder(), GetRemovedHolder()); }

			shared_ptr<IEnumerable<ValueType>> Reverse() const override
			{ return MakeSimpleEnumerable(Bind(MakeShared<SortedCollectionTransactionEnumerator<ReverseEnumerationDirection>>(), _impl->GetItemsHolder(), GetAddedHolder(), GetRemovedHolder())); }

			size_t GetCount() const override
			{ return _impl->Items->size() - _removed->size() + _added->size(); }

			bool IsEmpty() const override
			{ return _added->empty() && _impl->Items->size() == _removed->size(); }

			bool Contains(const ValueType& value) const override
			{ return _added->count(value) || (_impl->Items->count(value) && !_removed->count(value)); }

			shared_ptr<IEnumerator<ValueType>> Find(const ValueType& value) const override
			{
				if (Contains(value))
					return make_shared_ptr<SortedCollectionTransactionEnumerator<ForwardFindEnumerationDirection>>(_impl->GetItemsHolder(), GetAddedHolder(), GetRemovedHolder(), value);
				else
					return MakeEmptyEnumerator();
			}

			shared_ptr<IEnumerator<ValueType>> ReverseFind(const ValueType& value) const override
			{
				if (Contains(value))
					return make_shared_ptr<SortedCollectionTransactionEnumerator<ReverseFindEnumerationDirection>>(_impl->GetItemsHolder(), GetAddedHolder(), GetRemovedHolder(), value);
				else
					return MakeEmptyEnumerator();
			}

			void Add(const ValueType& value) override
			{
				if (_removed->count(value))
				{
					CopyRemovedOnWrite(_removed.get());
					_removed->erase(value);
				}
				else if (!_added->count(value) && !_impl->Items->count(value))
				{
					CopyAddedOnWrite(_added.get());
					_added->insert(value);
				}
			}

			void Remove(const ValueType& value) override
			{ TryRemove(value); }

			bool TryRemove(const ValueType& value) override
			{
				if (_added->count(value))
				{
					CopyAddedOnWrite(_added.get());
					_added->erase(value);
					return true;
				}
				else if (!_removed->count(value) && _impl->Items->count(value))
				{
					CopyRemovedOnWrite(_removed.get());
					_removed->insert(value);
					return true;
				}

				return false;
			}

			size_t RemoveWhere(const function<bool (const ValueType&)>& pred) override
			{
				CopyAddedOnWrite(_added.get());
				CopyRemovedOnWrite(_removed.get());

				size_t ret = 0;

				for (typename SetType::iterator it = _added->begin(); it != _added->end(); )
				{
					const typename SetType::iterator cur = it++;
					if (!pred(*cur))
						continue;

					_added->erase(cur);
					++ret;
				}

				for (const ValueType& value : *_impl->Items)
				{
					if (_removed->count(value) || !pred(value))
						continue;

					_removed->insert(value);
					++ret;
				}

				return ret;
			}

			void Clear() override
			{
				if (!CopyAddedOnWrite(null))
					_added->clear();

				if (!CopyRemovedOnWrite(_impl->Items.get()))
					*_removed = *_impl->Items;
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

			DiffTypePtr Diff() const override
			{ return MakeSimpleEnumerable(Bind(MakeShared<SortedCollectionTransactionDiffEnumerator<ForwardEnumerationDirection>>(), GetAddedHolder(), GetRemovedHolder())); }

			bool IsDirty() const override
			{ return !_added->empty() || !_removed->empty(); }

			void Commit() override
			{
				if (_added->empty() && _removed->empty())
					return;

				const DiffTypePtr diff = Diff();

				{
					MutexLock l(*_impl->Guard);
					Utils::CopyOnWrite(_impl->Items, _impl->ItemsHolder, _impl->Items.get());

					for (const ValueType& value : *_removed)
						_impl->Items->erase(value);
					for (const ValueType& value : *_added)
						STINGRAYKIT_CHECK(_impl->Items->insert(value).second, LogicException("Broken invariant: failed to add to items"));

					_impl->OnChanged(diff);
				}

				Revert();
			}

			void Revert() override
			{
				if (!CopyAddedOnWrite(null))
					_added->clear();

				if (!CopyRemovedOnWrite(null))
					_removed->clear();
			}

		private:
			HolderPtr GetAddedHolder() const
			{ return Utils::GetItemsHolder(_added, _addedHolder); }

			bool CopyAddedOnWrite(const SetType* src)
			{ return Utils::CopyOnWrite(_added, _addedHolder, src); }

			HolderPtr GetRemovedHolder() const
			{ return Utils::GetItemsHolder(_removed, _removedHolder); }

			bool CopyRemovedOnWrite(const SetType* src)
			{ return Utils::CopyOnWrite(_removed, _removedHolder, src); }
		};

	private:
		const ImplDataPtr					_impl;

	public:
		TransactionalSet()
			:	_impl(make_shared_ptr<ImplData>())
		{ }

		shared_ptr<IEnumerator<ValueType>> GetEnumerator() const override
		{
			MutexLock l(*_impl->Guard);
			return EnumeratorFromStlContainer(*_impl->Items, _impl->GetItemsHolder());
		}

		shared_ptr<IEnumerable<ValueType>> Reverse() const override
		{
			MutexLock l(*_impl->Guard);
			return make_shared_ptr<ReverseEnumerable>(_impl->GetItemsHolder());
		}

		size_t GetCount() const override
		{
			MutexLock l(*_impl->Guard);
			return _impl->Items->size();
		}

		bool IsEmpty() const override
		{
			MutexLock l(*_impl->Guard);
			return _impl->Items->empty();
		}

		bool Contains(const ValueType& value) const override
		{
			MutexLock l(*_impl->Guard);
			return _impl->Items->find(value) != _impl->Items->end();
		}

		shared_ptr<IEnumerator<ValueType>> Find(const ValueType& value) const override
		{
			MutexLock l(*_impl->Guard);

			const typename SetType::const_iterator it = _impl->Items->find(value);
			if (it == _impl->Items->end())
				return MakeEmptyEnumerator();

			return EnumeratorFromStlIterators(it, _impl->Items->end(), _impl->GetItemsHolder());
		}

		shared_ptr<IEnumerator<ValueType>> ReverseFind(const ValueType& value) const override
		{
			using cri = typename SetType::const_reverse_iterator;

			MutexLock l(*_impl->Guard);

			typename SetType::const_iterator it = _impl->Items->find(value);
			if (it == _impl->Items->end())
				return MakeEmptyEnumerator();

			return EnumeratorFromStlIterators(cri(++it), _impl->Items->rend(), _impl->GetItemsHolder());
		}

		TransactionTypePtr StartTransaction(const ICancellationToken& token = DummyCancellationToken()) override
		{ return make_shared_ptr<Transaction>(_impl, token); }

		signal_connector<void (const DiffTypePtr&)> OnChanged() const override
		{ return _impl->OnChanged.connector(); }

		const Mutex& GetSyncRoot() const override
		{ return *_impl->Guard; }
	};

}

#endif
