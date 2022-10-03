#ifndef STINGRAYKIT_COLLECTION_TRANSACTIONALLIST_H
#define STINGRAYKIT_COLLECTION_TRANSACTIONALLIST_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/ITransactionalList.h>
#include <stingraykit/diagnostics/ExecutorsProfiler.h>
#include <stingraykit/log/Logger.h>
#include <stingraykit/signal/signals.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename ValueType_, typename ValueEqualsComparer_ >
	class TransactionalList : public virtual ITransactionalList<ValueType_>
	{
		STINGRAYKIT_NONCOPYABLE(TransactionalList);

	private:
		using ExternalMutexPointer = signal_policies::threading::ExternalMutexPointer;

	public:
		using base = ITransactionalList<ValueType_>;

		using ValueType = typename base::ValueType;
		using PairType = typename base::PairType;

		using DiffEntryType = typename base::DiffEntryType;
		using DiffTypePtr = typename base::DiffTypePtr;

		using OnChangedSignature = typename base::OnChangedSignature;

		using TransactionTypePtr = typename base::TransactionTypePtr;

		using ValueEqualsComparer = ValueEqualsComparer_;

	private:
		using VectorType = std::vector<ValueType>;
		STINGRAYKIT_DECLARE_PTR(VectorType);
		STINGRAYKIT_DECLARE_CONST_PTR(VectorType);

		struct Utils
		{
			static DiffTypePtr MakeDiff(const VectorTypeConstPtr& oldItems, const VectorTypeConstPtr& newItems)
			{
				const shared_ptr<std::vector<DiffEntryType>> diff = make_shared_ptr<std::vector<DiffEntryType>>();

				const size_t oldSize = oldItems->size();
				const size_t newSize = newItems->size();

				for (size_t index = 0, tailIndex = 0; index < oldSize || index < newSize; ++index)
				{
					if (index >= newSize)
						diff->push_back(MakeDiffEntry(CollectionOp::Removed, PairType(index - tailIndex++, (*oldItems)[index])));
					else if (index >= oldSize)
						diff->push_back(MakeDiffEntry(CollectionOp::Added, PairType(index, (*newItems)[index])));
					else if (!ValueEqualsComparer()((*oldItems)[index], (*newItems)[index]))
					{
						// TODO: fix Updated handling
						//diff->push_back(MakeDiffEntry(CollectionOp::Updated, *newIt));
						diff->push_back(MakeDiffEntry(CollectionOp::Removed, PairType(index, (*oldItems)[index])));
						diff->push_back(MakeDiffEntry(CollectionOp::Added, PairType(index, (*newItems)[index])));
					}
				}

				return EnumerableFromStlContainer(*diff, diff);
			}

			static std::string GetProfilerMessage()
			{ return StringBuilder() % "Starting transaction on " % TypeInfo(typeid(TransactionalList)).GetClassName(); }
		};

		struct Holder
		{
			const VectorTypeConstPtr		Items;

			Holder(const VectorTypeConstPtr& items) : Items(items) { }
		};
		STINGRAYKIT_DECLARE_PTR(Holder);

		class ReverseEnumerable : public virtual IEnumerable<ValueType>
		{
		private:
			const HolderPtr					_holder;

		public:
			ReverseEnumerable(const HolderPtr& holder) : _holder(holder) { }

			shared_ptr<IEnumerator<ValueType>> GetEnumerator() const override
			{ return EnumeratorFromStlIterators(_holder->Items->rbegin(), _holder->Items->rend(), _holder); }
		};

		class PopulatorEnumerator : public virtual IEnumerator<DiffEntryType>
		{
			STINGRAYKIT_NONCOPYABLE(PopulatorEnumerator);

		private:
			using cit = typename VectorType::const_iterator;

		private:
			VectorTypeConstPtr	_items;
			cit					_current;
			size_t				_index;
			cit					_end;

		public:
			PopulatorEnumerator(const VectorTypeConstPtr& items)
				: _items(items), _current(_items->begin()), _index(0), _end(_items->end())
			{ }

			bool Valid() const override
			{ return _current != _end; }

			DiffEntryType Get() const override
			{
				STINGRAYKIT_CHECK(_current != _end, "Enumerator is not valid!");
				return MakeDiffEntry(CollectionOp::Added, PairType(_index, *_current));
			}

			void Next() override
			{
				STINGRAYKIT_CHECK(_current != _end, "Enumerator is not valid!");
				++_current;
				++_index;
			}
		};

		struct ImplData
		{
			shared_ptr<Mutex>											Guard;
			VectorTypeConstPtr											Items;
			bool														HasTransaction;
			ConditionVariable											TransactionCompleted;
			signal<OnChangedSignature, ExternalMutexPointer>			OnChanged;

		public:
			ImplData()
				:	Guard(make_shared_ptr<Mutex>()),
					Items(make_shared_ptr<VectorType>()),
					HasTransaction(false),
					OnChanged(ExternalMutexPointer(Guard), Bind(&ImplData::OnChangedPopulator, this, _1))
			{ }

		private:
			void OnChangedPopulator(const function<OnChangedSignature>& slot) const
			{
				if (!Items->empty())
					slot(MakeSimpleEnumerable(Bind(MakeShared<PopulatorEnumerator>(), Items)));
			}
		};
		STINGRAYKIT_DECLARE_PTR(ImplData);

		class Transaction : public virtual IListTransaction<ValueType>
		{
			STINGRAYKIT_NONCOPYABLE(Transaction);

		private:
			ImplDataPtr						_impl;
			VectorTypeConstPtr				_oldItems;
			VectorTypePtr					_newItems;
			mutable HolderWeakPtr			_newItemsHolder;
			mutable DiffTypePtr				_cachedDiff;

		public:
			Transaction(const ImplDataPtr& impl, const ICancellationToken& token)
				:	_impl(STINGRAYKIT_REQUIRE_NOT_NULL(impl))
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
				_oldItems = _impl->Items;
			}

			~Transaction() override
			{
				MutexLock l(*_impl->Guard);
				_impl->HasTransaction = false;
				STINGRAYKIT_TRY_NO_MESSAGE(_impl->TransactionCompleted.Broadcast());
			}

			shared_ptr<IEnumerator<ValueType>> GetEnumerator() const override
			{
				const HolderPtr holder = GetItemsHolder();
				return EnumeratorFromStlContainer(*holder->Items, holder);
			}

			shared_ptr<IEnumerable<ValueType>> Reverse() const override
			{ return make_shared_ptr<ReverseEnumerable>(GetItemsHolder()); }

			size_t GetCount() const override
			{ return _newItems ? _newItems->size() : _oldItems->size(); }

			bool IsEmpty() const override
			{ return _newItems ? _newItems->empty() : _oldItems->empty(); }

			bool Contains(const ValueType& value) const override
			{
				const VectorType& items = _newItems ? *_newItems : *_oldItems;
				return std::find(items.begin(), items.end(), value) != items.end();
			}

			optional<size_t> IndexOf(const ValueType& value) const override
			{
				const VectorType& items = _newItems ? *_newItems : *_oldItems;

				const typename VectorType::const_iterator it = std::find(items.begin(), items.end(), value);
				return it == items.end() ? null : make_optional_value(std::distance(items.begin(), it));
			}

			ValueType Get(size_t index) const override
			{
				const VectorType& items = _newItems ? *_newItems : *_oldItems;

				STINGRAYKIT_CHECK(index < items.size(), IndexOutOfRangeException(index, items.size()));
				return items[index];
			}

			bool TryGet(size_t index, ValueType& value) const override
			{
				const VectorType& items = _newItems ? *_newItems : *_oldItems;

				if (index >= items.size())
					return false;

				value = items[index];
				return true;
			}

			void Add(const ValueType& value) override
			{
				CopyOnWrite();
				_newItems->push_back(value);
			}

			void Set(size_t index, const ValueType& value) override
			{
				const VectorType& items = _newItems ? *_newItems : *_oldItems;
				STINGRAYKIT_CHECK(index < items.size(), IndexOutOfRangeException(index, items.size()));

				CopyOnWrite();
				(*_newItems)[index] = value;
			}

			void Insert(size_t index, const ValueType& value) override
			{
				const VectorType& items = _newItems ? *_newItems : *_oldItems;
				STINGRAYKIT_CHECK(index <= items.size(), IndexOutOfRangeException(index, items.size()));

				CopyOnWrite();
				_newItems->insert(std::next(_newItems->begin(), index), value);
			}

			void RemoveAt(size_t index) override
			{
				const VectorType& items = _newItems ? *_newItems : *_oldItems;
				STINGRAYKIT_CHECK(index < items.size(), IndexOutOfRangeException(index, items.size()));

				CopyOnWrite();
				_newItems->erase(std::next(_newItems->begin(), index));
			}

			bool TryRemove(const ValueType& value) override
			{
				const VectorType& items = _newItems ? *_newItems : *_oldItems;

				const typename VectorType::const_iterator it = std::find(items.begin(), items.end(), value);
				if (it == items.end())
					return false;

				const size_t index = std::distance(items.begin(), it);

				CopyOnWrite();
				_newItems->erase(std::next(_newItems->begin(), index));
				return true;
			}

			size_t RemoveAll(const function<bool (const ValueType&)>& pred) override
			{
				CopyOnWrite();
				const typename VectorType::iterator it = std::remove_if(_newItems->begin(), _newItems->end(), pred);
				const size_t ret = std::distance(it, _newItems->end());
				_newItems->erase(it, _newItems->end());
				return ret;
			}

			void Clear() override
			{
				_cachedDiff.reset();

				if (_newItems && _newItemsHolder.expired())
					_newItems->clear();
				else
				{
					_newItems = make_shared_ptr<VectorType>();
					_newItemsHolder.reset();
				}
			}

			void Apply(const DiffEntryType& entry) override
			{
				switch (entry.Op)
				{
				case CollectionOp::Added:
					Set(entry.Item.Key, entry.Item.Value);
					break;
				case CollectionOp::Updated:
					Set(entry.Item.Key, entry.Item.Value);
					break;
				case CollectionOp::Removed:
					RemoveAt(entry.Item.Key);
					break;
				}
			}

			void Commit() override
			{
				if (_newItems)
				{
					const DiffTypePtr diff = _cachedDiff ? _cachedDiff : Utils::MakeDiff(_oldItems, _newItems);
					if (!Enumerable::Any(diff))
					{
						ResetWrite();
						return;
					}

					{
						MutexLock l(*_impl->Guard);
						_impl->Items = _newItems;
						_impl->OnChanged(diff);
					}

					_oldItems = _newItems;
				}

				ResetWrite();
			}

			void Revert() override
			{ ResetWrite(); }

			DiffTypePtr Diff() const override
			{
				if (_cachedDiff)
					return _cachedDiff;

				return _newItems ? _cachedDiff = Utils::MakeDiff(_oldItems, _newItems) : MakeEmptyEnumerable();
			}

			bool IsDirty() const override
			{ return _newItems.is_initialized(); }

		private:
			HolderPtr GetItemsHolder() const
			{
				if (!_newItems)
					return make_shared_ptr<Holder>(_oldItems);

				HolderPtr newItemsHolder = _newItemsHolder.lock();

				if (!newItemsHolder)
					_newItemsHolder = (newItemsHolder = make_shared_ptr<Holder>(_newItems));

				return newItemsHolder;
			}

			void CopyOnWrite()
			{
				_cachedDiff.reset();

				if (!_newItems)
					_newItems = make_shared_ptr<VectorType>(*_oldItems);
				else if (!_newItemsHolder.expired())
				{
					_newItems = make_shared_ptr<VectorType>(*_newItems);
					_newItemsHolder.reset();
				}
			}

			void ResetWrite()
			{
				_cachedDiff.reset();
				_newItems.reset();
				_newItemsHolder.reset();
			}
		};

	private:
		ImplDataPtr							_impl;

	public:
		TransactionalList()
			:	_impl(make_shared_ptr<ImplData>())
		{ }

		shared_ptr<IEnumerator<ValueType>> GetEnumerator() const override
		{
			MutexLock l(*_impl->Guard);
			return EnumeratorFromStlContainer(*_impl->Items, _impl->Items);
		}

		shared_ptr<IEnumerable<ValueType>> Reverse() const override
		{
			MutexLock l(*_impl->Guard);
			return make_shared_ptr<ReverseEnumerable>(make_shared_ptr<Holder>(_impl->Items));
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
			return std::find(_impl->Items->begin(), _impl->Items->end(), value) != _impl->Items->end();
		}

		optional<size_t> IndexOf(const ValueType& value) const override
		{
			MutexLock l(*_impl->Guard);

			const typename VectorType::const_iterator it = std::find(_impl->Items->begin(), _impl->Items->end(), value);
			return it == _impl->Items->end() ? null : make_optional_value(std::distance(_impl->Items->begin(), it));
		}

		ValueType Get(size_t index) const override
		{
			MutexLock l(*_impl->Guard);

			STINGRAYKIT_CHECK(index < _impl->Items->size(), IndexOutOfRangeException(index, _impl->Items->size()));
			return (*_impl->Items)[index];
		}

		bool TryGet(size_t index, ValueType& value) const override
		{
			MutexLock l(*_impl->Guard);

			if (index >= _impl->Items->size())
				return false;

			value = (*_impl->Items)[index];
			return true;
		}

		TransactionTypePtr StartTransaction(const ICancellationToken& token = DummyCancellationToken()) override
		{ return make_shared_ptr<Transaction>(_impl, token); }

		signal_connector<OnChangedSignature> OnChanged() const override
		{ return _impl->OnChanged.connector(); }

		const Mutex& GetSyncRoot() const override
		{ return *_impl->Guard; }
	};

	/** @} */

}

#endif
