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
			static shared_ptr<IEnumerator<PairType>> WrapMapEnumerator(const shared_ptr<IEnumerator<MapValueType>>& mapEnumerator)
			{ return WrapEnumerator(mapEnumerator); }

			static DiffEntryType MakeDiffEntry(CollectionOp op, const MapValueType& pair)
			{ return stingray::MakeDiffEntry(op, PairType(pair)); }

			static DiffTypePtr MakeDiff(const MapTypeConstPtr& oldMap, const MapTypeConstPtr& newMap)
			{
				using cit = typename MapType::const_iterator;

				const shared_ptr<std::vector<DiffEntryType>> diff = make_shared_ptr<std::vector<DiffEntryType>>();

				const cit oldEnd = oldMap->end();
				const cit newEnd = newMap->end();

				cit oldIt = oldMap->begin();
				cit newIt = newMap->begin();

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
			MapTypeConstPtr		Map;
			Holder(const MapTypeConstPtr& map) : Map(map) { }
		};
		STINGRAYKIT_DECLARE_PTR(Holder);

		class ReverseEnumerable : public virtual IEnumerable<PairType>
		{
		private:
			HolderPtr		_holder;

		public:
			ReverseEnumerable(const HolderPtr& holder) : _holder(holder) { }

			shared_ptr<IEnumerator<PairType>> GetEnumerator() const override
			{ return Utils::WrapMapEnumerator(EnumeratorFromStlIterators(_holder->Map->rbegin(), _holder->Map->rend(), _holder)); }
		};

		struct ImplData
		{
			shared_ptr<Mutex>											Guard;
			MapTypeConstPtr												Map;
			bool														HasTransaction;
			ConditionVariable											TransactionCompleted;
			signal<void (const DiffTypePtr&), EMP>						OnChanged;

		public:
			ImplData()
				:	Guard(make_shared_ptr<Mutex>()),
					Map(make_shared_ptr<MapType>()),
					HasTransaction(false),
					OnChanged(EMP(Guard), Bind(&ImplData::OnChangedPopulator, this, _1))
			{ }

		private:
			void OnChangedPopulator(const function<void (const DiffTypePtr&)>& slot) const
			{ slot(WrapEnumerable(EnumerableFromStlContainer(*Map, Map), Bind(&Utils::MakeDiffEntry, CollectionOp::Added, _1))); }
		};
		STINGRAYKIT_DECLARE_PTR(ImplData);

		class Transaction : public virtual IDictionaryTransaction<KeyType, ValueType>
		{
			STINGRAYKIT_NONCOPYABLE(Transaction);

		private:
			ImplDataPtr						_impl;
			MapTypeConstPtr					_oldMap;
			MapTypePtr						_newMap;
			mutable HolderWeakPtr			_newMapHolder;
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
				_oldMap = _impl->Map;
			}

			~Transaction() override
			{
				MutexLock l(*_impl->Guard);
				_impl->HasTransaction = false;
				STINGRAYKIT_TRY_NO_MESSAGE(_impl->TransactionCompleted.Broadcast());
			}

			shared_ptr<IEnumerator<PairType>> GetEnumerator() const override
			{
				const HolderPtr holder = GetMapHolder();
				return Utils::WrapMapEnumerator(EnumeratorFromStlContainer(*holder->Map, holder));
			}

			shared_ptr<IEnumerable<PairType>> Reverse() const override
			{ return make_shared_ptr<ReverseEnumerable>(GetMapHolder()); }

			size_t GetCount() const override
			{ return _newMap ? _newMap->size() : _oldMap->size(); }

			bool IsEmpty() const override
			{ return _newMap ? _newMap->empty() : _oldMap->empty(); }

			bool ContainsKey(const KeyType& key) const override
			{
				const MapType& map = _newMap ? *_newMap : *_oldMap;
				return map.find(key) != map.end();
			}

			shared_ptr<IEnumerator<PairType>> Find(const KeyType& key) const override
			{
				const HolderPtr holder = GetMapHolder();

				const typename MapType::const_iterator it = holder->Map->find(key);
				if (it == holder->Map->end())
					return MakeEmptyEnumerator();

				return Utils::WrapMapEnumerator(EnumeratorFromStlIterators(it, holder->Map->end(), holder));
			}

			shared_ptr<IEnumerator<PairType>> ReverseFind(const KeyType& key) const override
			{
				using cri = typename MapType::const_reverse_iterator;

				const HolderPtr holder = GetMapHolder();

				typename MapType::const_iterator it = holder->Map->find(key);
				if (it == holder->Map->end())
					return MakeEmptyEnumerator();

				return Utils::WrapMapEnumerator(EnumeratorFromStlIterators(cri(++it), cri(holder->Map->rend()), holder));
			}

			ValueType Get(const KeyType& key) const override
			{
				const MapType& map = _newMap ? *_newMap : *_oldMap;

				const typename MapType::const_iterator it = map.find(key);
				STINGRAYKIT_CHECK(it != map.end(), CreateKeyNotFoundException(key));

				return it->second;
			}

			bool TryGet(const KeyType& key, ValueType& outValue) const override
			{
				const MapType& map = _newMap ? *_newMap : *_oldMap;

				const typename MapType::const_iterator it = map.find(key);
				if (it == map.end())
					return false;

				outValue = it->second;
				return true;
			}

			void Set(const KeyType& key, const ValueType& value) override
			{
				CopyOnWrite();

				const typename MapType::iterator it = _newMap->find(key);
				if (it != _newMap->end())
					it->second = value;
				else
					_newMap->insert(std::make_pair(key, value));
			}

			void Remove(const KeyType& key) override
			{ CopyOnWrite(); _newMap->erase(key); }

			bool TryRemove(const KeyType& key) override
			{
				const MapType& map = _newMap ? *_newMap : *_oldMap;

				const typename MapType::const_iterator it = map.find(key);
				if (it == map.end())
					return false;

				CopyOnWrite();
				_newMap->erase(key);
				return true;
			}

			size_t RemoveWhere(const function<bool (const KeyType&, const ValueType&)>& pred) override
			{
				CopyOnWrite();
				size_t ret = 0;
				for (typename MapType::iterator it = _newMap->begin(); it != _newMap->end(); )
				{
					const typename MapType::iterator cur = it++;
					if (!pred(cur->first, cur->second))
						continue;

					_newMap->erase(cur);
					++ret;
				}
				return ret;
			}

			void Clear() override
			{
				_cachedDiff.reset();

				if (_newMap && _newMapHolder.expired())
					_newMap->clear();
				else
				{
					_newMap = make_shared_ptr<MapType>();
					_newMapHolder.reset();
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

				return _newMap ? _cachedDiff = Utils::MakeDiff(_oldMap, _newMap) : MakeEmptyEnumerable();
			}

			bool IsDirty() const override
			{ return _newMap.is_initialized(); }

			void Commit() override
			{
				if (_newMap)
				{
					const DiffTypePtr diff = _cachedDiff ? _cachedDiff : Utils::MakeDiff(_oldMap, _newMap);
					if (!Enumerable::Any(diff))
					{
						ResetWrite();
						return;
					}

					{
						MutexLock l(*_impl->Guard);
						_impl->Map = _newMap;
						_impl->OnChanged(diff);
					}

					_oldMap = _newMap;
				}

				ResetWrite();
			}

			void Revert() override
			{ ResetWrite(); }

		private:
			HolderPtr GetMapHolder() const
			{
				if (!_newMap)
					return make_shared_ptr<Holder>(_oldMap);

				HolderPtr newMapHolder = _newMapHolder.lock();

				if (!newMapHolder)
					_newMapHolder = (newMapHolder = make_shared_ptr<Holder>(_newMap));

				return newMapHolder;
			}

			void CopyOnWrite()
			{
				_cachedDiff.reset();

				if (!_newMap)
					_newMap = make_shared_ptr<MapType>(*_oldMap);
				else if (!_newMapHolder.expired())
				{
					_newMap = make_shared_ptr<MapType>(*_newMap);
					_newMapHolder.reset();
				}
			}

			void ResetWrite()
			{
				_cachedDiff.reset();
				_newMap.reset();
				_newMapHolder.reset();
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
			return Utils::WrapMapEnumerator(EnumeratorFromStlContainer(*_impl->Map, _impl->Map));
		}

		shared_ptr<IEnumerable<PairType>> Reverse() const override
		{
			MutexLock l(*_impl->Guard);
			return make_shared_ptr<ReverseEnumerable>(make_shared_ptr<Holder>(_impl->Map));
		}

		size_t GetCount() const override
		{
			MutexLock l(*_impl->Guard);
			return _impl->Map->size();
		}

		bool IsEmpty() const override
		{
			MutexLock l(*_impl->Guard);
			return _impl->Map->empty();
		}

		bool ContainsKey(const KeyType& key) const override
		{
			MutexLock l(*_impl->Guard);
			return _impl->Map->find(key) != _impl->Map->end();
		}

		shared_ptr<IEnumerator<PairType>> Find(const KeyType& key) const override
		{
			MutexLock l(*_impl->Guard);

			const typename MapType::const_iterator it = _impl->Map->find(key);
			if (it == _impl->Map->end())
				return MakeEmptyEnumerator();

			return Utils::WrapMapEnumerator(EnumeratorFromStlIterators(it, _impl->Map->end(), _impl->Map));
		}

		shared_ptr<IEnumerator<PairType>> ReverseFind(const KeyType& key) const override
		{
			using cri = typename MapType::const_reverse_iterator;

			MutexLock l(*_impl->Guard);

			typename MapType::const_iterator it = _impl->Map->find(key);
			if (it == _impl->Map->end())
				return MakeEmptyEnumerator();

			return Utils::WrapMapEnumerator(EnumeratorFromStlIterators(cri(++it), cri(_impl->Map->rend()), _impl->Map));
		}

		ValueType Get(const KeyType& key) const override
		{
			MutexLock l(*_impl->Guard);

			const typename MapType::const_iterator it = _impl->Map->find(key);
			STINGRAYKIT_CHECK(it != _impl->Map->end(), CreateKeyNotFoundException(key));

			return it->second;
		}

		bool TryGet(const KeyType& key, ValueType& outValue) const override
		{
			MutexLock l(*_impl->Guard);

			const typename MapType::const_iterator it = _impl->Map->find(key);
			if (it == _impl->Map->end())
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
