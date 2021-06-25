#ifndef STINGRAYKIT_COLLECTION_TRANSACTIONALDICTIONARY_H
#define STINGRAYKIT_COLLECTION_TRANSACTIONALDICTIONARY_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/ITransactionalDictionary.h>
#include <stingraykit/collection/KeyNotFoundExceptionCreator.h>
#include <stingraykit/diagnostics/ExecutorsProfiler.h>
#include <stingraykit/signal/signals.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename KeyType_, typename ValueType_, typename KeyLessComparer_ = comparers::Less, typename ValueEqualsComparer_ = comparers::Equals >
	class TransactionalDictionary : public virtual ITransactionalDictionary<KeyType_, ValueType_>
	{
		STINGRAYKIT_NONCOPYABLE(TransactionalDictionary);

	private:
		using EMP = signal_policies::threading::ExternalMutexPointer;

	public:
		using base = ITransactionalDictionary<KeyType_, ValueType_>;

		using KeyType = typename base::KeyType;
		using ValueType = typename base::ValueType;
		using PairType = typename base::PairType;

		using DiffEntryType = typename base::DiffEntryType;
		using DiffTypePtr = typename base::DiffTypePtr;
		using TransactionTypePtr = typename base::TransactionTypePtr;

		using KeyLessComparer = KeyLessComparer_;
		using ValueEqualsComparer = ValueEqualsComparer_;

	private:
		using MapType = std::map<KeyType, ValueType, KeyLessComparer>;
		STINGRAYKIT_DECLARE_PTR(MapType);
		STINGRAYKIT_DECLARE_CONST_PTR(MapType);

		using MapValueType = typename MapType::value_type;

		struct Utils
		{
			static shared_ptr<IEnumerator<PairType>> WrapEnumerator(const shared_ptr<IEnumerator<MapValueType>>& itemsEnumerator)
			{ return stingray::WrapEnumerator(itemsEnumerator); }

			static DiffEntryType MakeDiffEntry(CollectionOp op, const MapValueType& pair)
			{ return stingray::MakeDiffEntry(op, PairType(pair)); }

			static DiffTypePtr MakeDiff(const MapTypeConstPtr& oldItems, const MapTypeConstPtr& newItems)
			{
				using cit = typename MapType::const_iterator;

				const shared_ptr<std::vector<DiffEntryType>> diff = make_shared_ptr<std::vector<DiffEntryType>>();

				const cit oldEnd = oldItems->end();
				const cit newEnd = newItems->end();

				cit oldIt = oldItems->begin();
				cit newIt = newItems->begin();

				while (oldIt != oldEnd || newIt != newEnd)
				{
					if (newIt == newEnd)
					{
						diff->push_back(MakeDiffEntry(CollectionOp::Removed, *oldIt));
						++oldIt;
					}
					else if (oldIt == oldEnd || KeyLessComparer()(newIt->first, oldIt->first))
					{
						diff->push_back(MakeDiffEntry(CollectionOp::Added, *newIt));
						++newIt;
					}
					else if (KeyLessComparer()(oldIt->first, newIt->first))
					{
						diff->push_back(MakeDiffEntry(CollectionOp::Removed, *oldIt));
						++oldIt;
					}
					else // oldIt->first == newIt->first
					{
						if (!ValueEqualsComparer()(oldIt->second, newIt->second))
						{
							// TODO: fix Updated handling
							//diff->push_back(MakeDiffEntry(CollectionOp::Updated, *newIt));
							diff->push_back(MakeDiffEntry(CollectionOp::Removed, *oldIt));
							diff->push_back(MakeDiffEntry(CollectionOp::Added, *newIt));
						}
						++oldIt;
						++newIt;
					}
				}

				return EnumerableFromStlContainer(*diff, diff);
			}

			static std::string GetProfilerMessage()
			{ return StringBuilder() % "Starting transaction on " % TypeInfo(typeid(TransactionalDictionary)).GetClassName(); }
		};

		struct Holder
		{
			MapTypeConstPtr		Items;
			Holder(const MapTypeConstPtr& items) : Items(items) { }
		};
		STINGRAYKIT_DECLARE_PTR(Holder);

		class ReverseEnumerable : public virtual IEnumerable<PairType>
		{
		private:
			HolderPtr		_holder;

		public:
			ReverseEnumerable(const HolderPtr& holder) : _holder(holder) { }

			shared_ptr<IEnumerator<PairType>> GetEnumerator() const override
			{ return Utils::WrapEnumerator(EnumeratorFromStlIterators(_holder->Items->rbegin(), _holder->Items->rend(), _holder)); }
		};

		struct ImplData
		{
			shared_ptr<Mutex>											Guard;
			MapTypeConstPtr												Items;
			bool														HasTransaction;
			ConditionVariable											TransactionCompleted;
			signal<void (const DiffTypePtr&), EMP>						OnChanged;

		public:
			ImplData()
				:	Guard(make_shared_ptr<Mutex>()),
					Items(make_shared_ptr<MapType>()),
					HasTransaction(false),
					OnChanged(EMP(Guard), Bind(&ImplData::OnChangedPopulator, this, _1))
			{ }

		private:
			void OnChangedPopulator(const function<void (const DiffTypePtr&)>& slot) const
			{ slot(WrapEnumerable(EnumerableFromStlContainer(*Items, Items), Bind(&Utils::MakeDiffEntry, CollectionOp::Added, _1))); }
		};
		STINGRAYKIT_DECLARE_PTR(ImplData);

		class Transaction : public virtual IDictionaryTransaction<KeyType, ValueType>
		{
			STINGRAYKIT_NONCOPYABLE(Transaction);

		private:
			ImplDataPtr						_impl;
			MapTypeConstPtr					_oldItems;
			MapTypePtr						_newItems;
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

			shared_ptr<IEnumerator<PairType>> GetEnumerator() const override
			{
				const HolderPtr holder = GetItemsHolder();
				return Utils::WrapEnumerator(EnumeratorFromStlContainer(*holder->Items, holder));
			}

			shared_ptr<IEnumerable<PairType>> Reverse() const override
			{ return make_shared_ptr<ReverseEnumerable>(GetItemsHolder()); }

			size_t GetCount() const override
			{ return _newItems ? _newItems->size() : _oldItems->size(); }

			bool IsEmpty() const override
			{ return _newItems ? _newItems->empty() : _oldItems->empty(); }

			bool ContainsKey(const KeyType& key) const override
			{
				const MapType& items = _newItems ? *_newItems : *_oldItems;
				return items.find(key) != items.end();
			}

			shared_ptr<IEnumerator<PairType>> Find(const KeyType& key) const override
			{
				const HolderPtr holder = GetItemsHolder();

				const typename MapType::const_iterator it = holder->Items->find(key);
				if (it == holder->Items->end())
					return MakeEmptyEnumerator();

				return Utils::WrapEnumerator(EnumeratorFromStlIterators(it, holder->Items->end(), holder));
			}

			shared_ptr<IEnumerator<PairType>> ReverseFind(const KeyType& key) const override
			{
				using cri = typename MapType::const_reverse_iterator;

				const HolderPtr holder = GetItemsHolder();

				typename MapType::const_iterator it = holder->Items->find(key);
				if (it == holder->Items->end())
					return MakeEmptyEnumerator();

				return Utils::WrapEnumerator(EnumeratorFromStlIterators(cri(++it), cri(holder->Items->rend()), holder));
			}

			ValueType Get(const KeyType& key) const override
			{
				const MapType& items = _newItems ? *_newItems : *_oldItems;

				const typename MapType::const_iterator it = items.find(key);
				STINGRAYKIT_CHECK(it != items.end(), CreateKeyNotFoundException(key));

				return it->second;
			}

			bool TryGet(const KeyType& key, ValueType& outValue) const override
			{
				const MapType& items = _newItems ? *_newItems : *_oldItems;

				const typename MapType::const_iterator it = items.find(key);
				if (it == items.end())
					return false;

				outValue = it->second;
				return true;
			}

			void Set(const KeyType& key, const ValueType& value) override
			{
				CopyOnWrite();

				const typename MapType::iterator it = _newItems->find(key);
				if (it != _newItems->end())
					it->second = value;
				else
					_newItems->insert(std::make_pair(key, value));
			}

			void Remove(const KeyType& key) override
			{ CopyOnWrite(); _newItems->erase(key); }

			bool TryRemove(const KeyType& key) override
			{
				const MapType& items = _newItems ? *_newItems : *_oldItems;

				const typename MapType::const_iterator it = items.find(key);
				if (it == items.end())
					return false;

				CopyOnWrite();
				_newItems->erase(key);
				return true;
			}

			size_t RemoveWhere(const function<bool (const KeyType&, const ValueType&)>& pred) override
			{
				CopyOnWrite();
				size_t ret = 0;
				for (typename MapType::iterator it = _newItems->begin(); it != _newItems->end(); )
				{
					const typename MapType::iterator cur = it++;
					if (!pred(cur->first, cur->second))
						continue;

					_newItems->erase(cur);
					++ret;
				}
				return ret;
			}

			void Clear() override
			{
				_cachedDiff.reset();

				if (_newItems && _newItemsHolder.expired())
					_newItems->clear();
				else
				{
					_newItems = make_shared_ptr<MapType>();
					_newItemsHolder.reset();
				}
			}

			void Apply(const DiffEntryType& entry) override
			{
				switch (entry.Op)
				{
				case CollectionOp::Added:		Set(entry.Item.Key, entry.Item.Value); break;
				case CollectionOp::Removed:		Remove(entry.Item.Key); break;
				case CollectionOp::Updated:		Set(entry.Item.Key, entry.Item.Value); break;
				}
			}

			DiffTypePtr Diff() const override
			{
				if (_cachedDiff)
					return _cachedDiff;

				return _newItems ? _cachedDiff = Utils::MakeDiff(_oldItems, _newItems) : MakeEmptyEnumerable();
			}

			bool IsDirty() const override
			{ return _newItems.is_initialized(); }

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
					_newItems = make_shared_ptr<MapType>(*_oldItems);
				else if (!_newItemsHolder.expired())
				{
					_newItems = make_shared_ptr<MapType>(*_newItems);
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
		TransactionalDictionary()
			:	_impl(make_shared_ptr<ImplData>())
		{ }

		shared_ptr<IEnumerator<PairType>> GetEnumerator() const override
		{
			MutexLock l(*_impl->Guard);
			return Utils::WrapEnumerator(EnumeratorFromStlContainer(*_impl->Items, _impl->Items));
		}

		shared_ptr<IEnumerable<PairType>> Reverse() const override
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

		bool ContainsKey(const KeyType& key) const override
		{
			MutexLock l(*_impl->Guard);
			return _impl->Items->find(key) != _impl->Items->end();
		}

		shared_ptr<IEnumerator<PairType>> Find(const KeyType& key) const override
		{
			MutexLock l(*_impl->Guard);

			const typename MapType::const_iterator it = _impl->Items->find(key);
			if (it == _impl->Items->end())
				return MakeEmptyEnumerator();

			return Utils::WrapEnumerator(EnumeratorFromStlIterators(it, _impl->Items->end(), _impl->Items));
		}

		shared_ptr<IEnumerator<PairType>> ReverseFind(const KeyType& key) const override
		{
			using cri = typename MapType::const_reverse_iterator;

			MutexLock l(*_impl->Guard);

			typename MapType::const_iterator it = _impl->Items->find(key);
			if (it == _impl->Items->end())
				return MakeEmptyEnumerator();

			return Utils::WrapEnumerator(EnumeratorFromStlIterators(cri(++it), cri(_impl->Items->rend()), _impl->Items));
		}

		ValueType Get(const KeyType& key) const override
		{
			MutexLock l(*_impl->Guard);

			const typename MapType::const_iterator it = _impl->Items->find(key);
			STINGRAYKIT_CHECK(it != _impl->Items->end(), CreateKeyNotFoundException(key));

			return it->second;
		}

		bool TryGet(const KeyType& key, ValueType& outValue) const override
		{
			MutexLock l(*_impl->Guard);

			const typename MapType::const_iterator it = _impl->Items->find(key);
			if (it == _impl->Items->end())
				return false;

			outValue = it->second;
			return true;
		}

		TransactionTypePtr StartTransaction(const ICancellationToken& token = DummyCancellationToken()) override
		{ return make_shared_ptr<Transaction>(_impl, token); }

		signal_connector<void (const DiffTypePtr&)> OnChanged() const override
		{ return _impl->OnChanged.connector(); }

		const Mutex& GetSyncRoot() const override
		{ return *_impl->Guard; }
	};

	/** @} */

}

#endif
