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
			static HolderPtr GetItemsHolder(const SetTypePtr& items, HolderWeakPtr& itemsHolder_)
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

		class TransactionEnumerator : public virtual IEnumerator<ValueType>
		{
			STINGRAYKIT_NONCOPYABLE(TransactionEnumerator);

		private:
			using IteratorType = typename SetType::const_iterator;

			struct EntryType
			{
				STINGRAYKIT_ENUM_VALUES(None, Items, Added, Removed);
				STINGRAYKIT_DECLARE_ENUM_CLASS(EntryType);
			};

		private:
			const HolderPtr			_itemsHolder;
			const HolderPtr			_addedHolder;
			const HolderPtr			_removedHolder;

			EntryType				_entryType;
			IteratorType			_itemsIt;
			IteratorType			_addedIt;
			IteratorType			_removedIt;

		public:
			TransactionEnumerator(const HolderPtr& itemsHolder, const HolderPtr& addedHolder, const HolderPtr& removedHolder)
				:	_itemsHolder(STINGRAYKIT_REQUIRE_NOT_NULL(itemsHolder)),
					_addedHolder(STINGRAYKIT_REQUIRE_NOT_NULL(addedHolder)),
					_removedHolder(STINGRAYKIT_REQUIRE_NOT_NULL(removedHolder)),
					_itemsIt(_itemsHolder->Items->begin()),
					_addedIt(_addedHolder->Items->begin()),
					_removedIt(_removedHolder->Items->begin())
			{
				UpdateEntryType();
				if (_entryType == EntryType::Removed)
					Next();
			}

			bool Valid() const override
			{ return _entryType == EntryType::Items || _entryType == EntryType::Added; }

			ValueType Get() const override
			{
				switch (_entryType)
				{
				case EntryType::Items:		return *_itemsIt;
				case EntryType::Added:		return *_addedIt;
				default:
					break;
				}

				STINGRAYKIT_THROW("Enumerator is not valid!");
			}

			void Next() override
			{
				do {
					switch (_entryType)
					{
					case EntryType::Items:		++_itemsIt; break;
					case EntryType::Added:		++_addedIt; break;
					case EntryType::Removed:		++_itemsIt; ++_removedIt; break;
					default:
						STINGRAYKIT_THROW("Enumerator is not valid!");
					}

					UpdateEntryType();
				} while (_entryType == EntryType::Removed);
			}

		private:
			void UpdateEntryType()
			{
				if (_itemsIt == _itemsHolder->Items->end())
				{
					_entryType = _addedIt != _addedHolder->Items->end() ? EntryType::Added : EntryType::None;
					return;
				}

				if (_addedIt != _addedHolder->Items->end() && ValueLessComparer()(*_addedIt, *_itemsIt))
				{
					_entryType = EntryType::Added;
					return;
				}

				if (_removedIt != _removedHolder->Items->end())
				{
					if (ValueLessComparer()(*_itemsIt, *_removedIt))
					{
						_entryType = EntryType::Items;
						return;
					}

					STINGRAYKIT_CHECK(!ValueLessComparer()(*_removedIt, *_itemsIt), LogicException("Broken invariant: 'removed' must be equals to 'item'"));
					_entryType = EntryType::Removed;
					return;
				}

				_entryType = EntryType::Items;
			}
		};

		class TransactionDiffEnumerator : public virtual IEnumerator<DiffEntryType>
		{
			STINGRAYKIT_NONCOPYABLE(TransactionDiffEnumerator);

		private:
			using IteratorType = typename SetType::const_iterator;

			struct EntryType
			{
				STINGRAYKIT_ENUM_VALUES(None, Added, Removed);
				STINGRAYKIT_DECLARE_ENUM_CLASS(EntryType);
			};

		private:
			const HolderPtr			_addedHolder;
			const HolderPtr			_removedHolder;

			EntryType				_entryType;
			IteratorType			_addedIt;
			IteratorType			_removedIt;

		public:
			TransactionDiffEnumerator(const HolderPtr& addedHolder, const HolderPtr& removedHolder)
				:	_addedHolder(STINGRAYKIT_REQUIRE_NOT_NULL(addedHolder)),
					_removedHolder(STINGRAYKIT_REQUIRE_NOT_NULL(removedHolder)),
					_addedIt(_addedHolder->Items->begin()),
					_removedIt(_removedHolder->Items->begin())
			{ UpdateEntryType(); }

			bool Valid() const override
			{ return _entryType != EntryType::None; }

			DiffEntryType Get() const override
			{
				switch (_entryType)
				{
				case EntryType::Added:		return MakeDiffEntry(CollectionOp::Added, *_addedIt);
				case EntryType::Removed:	return MakeDiffEntry(CollectionOp::Removed, *_removedIt);
				default:
					break;
				}

				STINGRAYKIT_THROW("Enumerator is not valid!");
			}

			void Next() override
			{
				switch (_entryType)
				{
				case EntryType::Added:		++_addedIt; break;
				case EntryType::Removed:	++_removedIt; break;
				default:
					STINGRAYKIT_THROW("Enumerator is not valid!");
				}

				UpdateEntryType();
			}

		private:
			void UpdateEntryType()
			{
				if (_addedIt != _addedHolder->Items->end())
				{
					if (_removedIt != _removedHolder->Items->end())
						_entryType = ValueLessComparer()(*_addedIt, *_removedIt) ? EntryType::Added : EntryType::Removed;
					else
						_entryType = EntryType::Added;
				}
				else if (_removedIt != _removedHolder->Items->end())
					_entryType = EntryType::Removed;
				else
					_entryType = EntryType::None;
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
			{ return make_shared_ptr<TransactionEnumerator>(_impl->GetItemsHolder(), GetAddedHolder(), GetRemovedHolder()); }

			shared_ptr<IEnumerable<ValueType>> Reverse() const override
			{ STINGRAYKIT_THROW(NotImplementedException()); }

			size_t GetCount() const override
			{ return _impl->Items->size() - _removed->size() + _added->size(); }

			bool IsEmpty() const override
			{ return _impl->Items->size() == _removed->size() && _added->empty(); }

			bool Contains(const ValueType& value) const override
			{ return (_impl->Items->count(value) && !_removed->count(value)) || _added->count(value); }

			shared_ptr<IEnumerator<ValueType>> Find(const ValueType& value) const override
			{ STINGRAYKIT_THROW(NotImplementedException()); }

			shared_ptr<IEnumerator<ValueType>> ReverseFind(const ValueType& value) const override
			{ STINGRAYKIT_THROW(NotImplementedException()); }

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
					if (!pred(value))
						continue;

					if (_removed->insert(value).second)
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
			{ return MakeSimpleEnumerable(Bind(MakeShared<TransactionDiffEnumerator>(), GetAddedHolder(), GetRemovedHolder())); }

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
						_impl->Items->insert(value);

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
		ImplDataPtr							_impl;

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
		{ STINGRAYKIT_THROW(NotImplementedException()); }

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
		{ STINGRAYKIT_THROW(NotImplementedException()); }

		shared_ptr<IEnumerator<ValueType>> ReverseFind(const ValueType& value) const override
		{ STINGRAYKIT_THROW(NotImplementedException()); }

		TransactionTypePtr StartTransaction(const ICancellationToken& token = DummyCancellationToken()) override
		{ return make_shared_ptr<Transaction>(_impl, token); }

		signal_connector<void (const DiffTypePtr&)> OnChanged() const override
		{ return _impl->OnChanged.connector(); }

		const Mutex& GetSyncRoot() const override
		{ return *_impl->Guard; }
	};

}

#endif
