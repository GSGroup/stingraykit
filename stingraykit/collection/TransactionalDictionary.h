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
#include <stingraykit/collection/TransactionHelpers.h>
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

		using OnChangedSignature = typename base::OnChangedSignature;

		using TransactionTypePtr = typename base::TransactionTypePtr;

		using KeyLessComparer = KeyLessComparer_;
		using ValueEqualsComparer = ValueEqualsComparer_;

	private:
		using MapType = std::map<KeyType, ValueType, KeyLessComparer>;
		STINGRAYKIT_DECLARE_PTR(MapType);
		STINGRAYKIT_DECLARE_CONST_PTR(MapType);

		using MapValueType = typename MapType::value_type;

		struct Holder
		{
			const MapTypeConstPtr		Items;

			Holder(const MapTypeConstPtr& items)
				:	Items(STINGRAYKIT_REQUIRE_NOT_NULL(items))
			{ }
		};
		STINGRAYKIT_DECLARE_PTR(Holder);

		struct Utils
		{
			static shared_ptr<IEnumerator<PairType>> WrapEnumerator(const shared_ptr<IEnumerator<MapValueType>>& itemsEnumerator)
			{ return stingray::WrapEnumerator(itemsEnumerator); }

			static DiffEntryType MakeDiffEntry(CollectionOp op, const MapValueType& pair)
			{ return stingray::MakeDiffEntry(op, PairType(pair)); }

			static HolderPtr GetItemsHolder(const MapTypeConstPtr& items, HolderWeakPtr& itemsHolder_)
			{
				HolderPtr itemsHolder = itemsHolder_.lock();

				if (!itemsHolder)
					itemsHolder_ = (itemsHolder = make_shared_ptr<Holder>(items));

				return itemsHolder;
			}

			static bool CopyOnWrite(MapTypePtr& items, HolderWeakPtr& itemsHolder, const MapType* src)
			{
				if (itemsHolder.expired())
					return false;

				items = src ? make_shared_ptr<MapType>(*src) : make_shared_ptr<MapType>();
				itemsHolder.reset();
				return true;
			}

			static std::string GetProfilerMessage()
			{ return StringBuilder() % "Starting transaction on " % TypeInfo(typeid(TransactionalDictionary)).GetClassName(); }
		};

		class ReverseEnumerable : public virtual IEnumerable<PairType>
		{
		private:
			const HolderPtr				_holder;

		public:
			ReverseEnumerable(const HolderPtr& holder)
				:	_holder(STINGRAYKIT_REQUIRE_NOT_NULL(holder))
			{ }

			shared_ptr<IEnumerator<PairType>> GetEnumerator() const override
			{ return Utils::WrapEnumerator(EnumeratorFromStlIterators(_holder->Items->rbegin(), _holder->Items->rend(), _holder)); }
		};

		struct ForwardEnumerationDirection
		{
			using ItemType = PairType;

			using CollectionType = Holder;
			using IteratorType = typename MapType::const_iterator;

			struct LessComparer : public function_info<bool, UnspecifiedParamTypes>
			{
				bool operator () (const MapValueType& lhs, const MapValueType& rhs) const
				{ return KeyLessComparer()(lhs.first, rhs.first); }
			};

			static IteratorType Begin(const CollectionType& holder)
			{ return holder.Items->begin(); }

			static IteratorType End(const CollectionType& holder)
			{ return holder.Items->end(); }

			static ItemType ToItem(const MapValueType& value)
			{ return ItemType(value); }
		};

		struct ForwardFindEnumerationDirection : public ForwardEnumerationDirection
		{
			using base = ForwardEnumerationDirection;

			const KeyType			Key;

			explicit ForwardFindEnumerationDirection(const KeyType& key) : Key(key) { }

			typename base::IteratorType Begin(const typename base::CollectionType& holder)
			{ return holder.Items->lower_bound(Key); }
		};

		struct ReverseEnumerationDirection
		{
			using ItemType = PairType;

			using CollectionType = Holder;
			using IteratorType = typename MapType::const_reverse_iterator;

			struct LessComparer : public function_info<bool, UnspecifiedParamTypes>
			{
				bool operator () (const MapValueType& lhs, const MapValueType& rhs) const
				{ return KeyLessComparer()(rhs.first, lhs.first); }
			};

			static IteratorType Begin(const CollectionType& holder)
			{ return holder.Items->rbegin(); }

			static IteratorType End(const CollectionType& holder)
			{ return holder.Items->rend(); }

			static ItemType ToItem(const MapValueType& value)
			{ return ItemType(value); }
		};

		struct ReverseFindEnumerationDirection : public ReverseEnumerationDirection
		{
			using base = ReverseEnumerationDirection;
			using IteratorType = typename base::IteratorType;

			const KeyType			Key;

			explicit ReverseFindEnumerationDirection(const KeyType& key) : Key(key) { }

			IteratorType Begin(const typename base::CollectionType& holder)
			{
				typename MapType::const_iterator itemsIt = holder.Items->lower_bound(Key);

				if (itemsIt == holder.Items->end() || KeyLessComparer()(Key, itemsIt->first))
					return IteratorType(itemsIt);

				return IteratorType(++itemsIt);
			}
		};

		struct ImplData
		{
			const shared_ptr<Mutex>						Guard;
			MapTypePtr									Items;
			mutable HolderWeakPtr						ItemsHolder;

			bool										HasTransaction;
			ConditionVariable							TransactionCompleted;
			signal<OnChangedSignature, EMP>				OnChanged;

		public:
			ImplData()
				:	Guard(make_shared_ptr<Mutex>()),
					Items(make_shared_ptr<MapType>()),
					HasTransaction(false),
					OnChanged(EMP(Guard), Bind(&ImplData::OnChangedPopulator, this, _1))
			{ }

			HolderPtr GetItemsHolder() const
			{
				MutexLock l(*Guard);
				return Utils::GetItemsHolder(Items, ItemsHolder);
			}

		private:
			void OnChangedPopulator(const function<OnChangedSignature>& slot) const
			{
				if (!Items->empty())
					slot(WrapEnumerable(EnumerableFromStlContainer(*Items, GetItemsHolder()), Bind(&Utils::MakeDiffEntry, CollectionOp::Added, _1)));
			}
		};
		STINGRAYKIT_DECLARE_PTR(ImplData);

		class Transaction : public virtual IDictionaryTransaction<KeyType, ValueType>
		{
			STINGRAYKIT_NONCOPYABLE(Transaction);

		private:
			ImplDataPtr						_impl;
			MapTypePtr						_added;
			mutable HolderWeakPtr			_addedHolder;
			MapTypePtr						_removed;
			mutable HolderWeakPtr			_removedHolder;

		public:
			Transaction(const ImplDataPtr& impl, const ICancellationToken& token)
				:	_impl(STINGRAYKIT_REQUIRE_NOT_NULL(impl)),
					_added(make_shared_ptr<MapType>()),
					_removed(make_shared_ptr<MapType>())
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

			shared_ptr<IEnumerator<PairType>> GetEnumerator() const override
			{ return make_shared_ptr<SortedCollectionTransactionEnumerator<ForwardEnumerationDirection>>(_impl->GetItemsHolder(), GetAddedHolder(), GetRemovedHolder()); }

			shared_ptr<IEnumerable<PairType>> Reverse() const override
			{ return MakeSimpleEnumerable(Bind(MakeShared<SortedCollectionTransactionEnumerator<ReverseEnumerationDirection>>(), _impl->GetItemsHolder(), GetAddedHolder(), GetRemovedHolder())); }

			size_t GetCount() const override
			{ return _impl->Items->size() - _removed->size() + _added->size(); }

			bool IsEmpty() const override
			{ return _added->empty() && _impl->Items->size() == _removed->size(); }

			bool ContainsKey(const KeyType& key) const override
			{ return _added->count(key) || (_impl->Items->count(key) && !_removed->count(key)); }

			shared_ptr<IEnumerator<PairType>> Find(const KeyType& key) const override
			{
				if (ContainsKey(key))
					return make_shared_ptr<SortedCollectionTransactionEnumerator<ForwardFindEnumerationDirection>>(_impl->GetItemsHolder(), GetAddedHolder(), GetRemovedHolder(), key);
				else
					return MakeEmptyEnumerator();
			}

			shared_ptr<IEnumerator<PairType>> ReverseFind(const KeyType& key) const override
			{
				if (ContainsKey(key))
					return make_shared_ptr<SortedCollectionTransactionEnumerator<ReverseFindEnumerationDirection>>(_impl->GetItemsHolder(), GetAddedHolder(), GetRemovedHolder(), key);
				else
					return MakeEmptyEnumerator();
			}

			ValueType Get(const KeyType& key) const override
			{
				const typename MapType::const_iterator addedIt = _added->find(key);
				if (addedIt != _added->end())
					return addedIt->second;

				const typename MapType::const_iterator itemsIt = _impl->Items->find(key);
				if (itemsIt != _impl->Items->end() && !_removed->count(key))
					return itemsIt->second;

				STINGRAYKIT_THROW(CreateKeyNotFoundException(key));
			}

			bool TryGet(const KeyType& key, ValueType& outValue) const override
			{
				const typename MapType::const_iterator addedIt = _added->find(key);
				if (addedIt != _added->end())
				{
					outValue = addedIt->second;
					return true;
				}

				const typename MapType::const_iterator itemsIt = _impl->Items->find(key);
				if (itemsIt != _impl->Items->end() && !_removed->count(key))
				{
					outValue = itemsIt->second;
					return true;
				}

				return false;
			}

			void Set(const KeyType& key, const ValueType& value) override
			{
				const typename MapType::const_iterator itemsIt = _impl->Items->find(key);
				if (itemsIt != _impl->Items->end())
				{
					if (ValueEqualsComparer()(value, itemsIt->second))
					{
						if (_added->count(key))
						{
							CopyAddedOnWrite(_added.get());
							_added->erase(key);
						}

						if (_removed->count(key))
						{
							CopyRemovedOnWrite(_removed.get());
							_removed->erase(key);
						}

						return;
					}

					if (!_removed->count(itemsIt->first))
					{
						CopyRemovedOnWrite(_removed.get());
						_removed->emplace(itemsIt->first, itemsIt->second);
					}
				}

				const typename MapType::const_iterator addedIt = _added->find(key);
				if (addedIt != _added->end())
				{
					if (!ValueEqualsComparer()(value, addedIt->second))
					{
						CopyAddedOnWrite(_added.get());
						(*_added)[key] = value;
					}
				}
				else
				{
					CopyAddedOnWrite(_added.get());
					_added->emplace(key, value);
				}
			}

			void Remove(const KeyType& key) override
			{ TryRemove(key); }

			bool TryRemove(const KeyType& key) override
			{
				if (_added->count(key))
				{
					CopyAddedOnWrite(_added.get());
					_added->erase(key);
					return true;
				}

				if (_removed->count(key))
					return false;

				const typename MapType::const_iterator itemsIt = _impl->Items->find(key);
				if (itemsIt != _impl->Items->end())
				{
					CopyRemovedOnWrite(_removed.get());
					_removed->emplace(itemsIt->first, itemsIt->second);
					return true;
				}

				return false;
			}

			size_t RemoveWhere(const function<bool (const KeyType&, const ValueType&)>& pred) override
			{
				CopyAddedOnWrite(_added.get());
				CopyRemovedOnWrite(_removed.get());

				size_t ret = 0;

				for (typename MapType::iterator it = _added->begin(); it != _added->end(); )
				{
					const typename MapType::iterator cur = it++;
					if (!pred(cur->first, cur->second))
						continue;

					_added->erase(cur);
					++ret;
				}

				for (const MapValueType& pair : *_impl->Items)
				{
					if (_removed->count(pair.first) || !pred(pair.first, pair.second))
						continue;

					_removed->emplace(pair.first, pair.second);
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
				case CollectionOp::Added:		Set(entry.Item.Key, entry.Item.Value); break;
				case CollectionOp::Removed:		Remove(entry.Item.Key); break;
				case CollectionOp::Updated:		Set(entry.Item.Key, entry.Item.Value); break;
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

					for (const MapValueType& pair : *_removed)
						_impl->Items->erase(pair.first);
					for (const MapValueType& pair : *_added)
						STINGRAYKIT_CHECK(_impl->Items->emplace(pair.first, pair.second).second, LogicException("Broken invariant: failed to add to items"));

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

			bool CopyAddedOnWrite(const MapType* src)
			{ return Utils::CopyOnWrite(_added, _addedHolder, src); }

			HolderPtr GetRemovedHolder() const
			{ return Utils::GetItemsHolder(_removed, _removedHolder); }

			bool CopyRemovedOnWrite(const MapType* src)
			{ return Utils::CopyOnWrite(_removed, _removedHolder, src); }
		};

	private:
		const ImplDataPtr					_impl;

	public:
		TransactionalDictionary()
			:	_impl(make_shared_ptr<ImplData>())
		{ }

		shared_ptr<IEnumerator<PairType>> GetEnumerator() const override
		{
			MutexLock l(*_impl->Guard);
			return Utils::WrapEnumerator(EnumeratorFromStlContainer(*_impl->Items, _impl->GetItemsHolder()));
		}

		shared_ptr<IEnumerable<PairType>> Reverse() const override
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

			return Utils::WrapEnumerator(EnumeratorFromStlIterators(it, _impl->Items->cend(), _impl->GetItemsHolder()));
		}

		shared_ptr<IEnumerator<PairType>> ReverseFind(const KeyType& key) const override
		{
			using cri = typename MapType::const_reverse_iterator;

			MutexLock l(*_impl->Guard);

			typename MapType::const_iterator it = _impl->Items->find(key);
			if (it == _impl->Items->end())
				return MakeEmptyEnumerator();

			return Utils::WrapEnumerator(EnumeratorFromStlIterators(cri(++it), _impl->Items->crend(), _impl->GetItemsHolder()));
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

		signal_connector<OnChangedSignature> OnChanged() const override
		{ return _impl->OnChanged.connector(); }

		const Mutex& GetSyncRoot() const override
		{ return *_impl->Guard; }
	};

	/** @} */

}

#endif
