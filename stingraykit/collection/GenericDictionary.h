#ifndef STINGRAYKIT_COLLECTION_GENERICDICTIONARY_H
#define STINGRAYKIT_COLLECTION_GENERICDICTIONARY_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/ForEach.h>
#include <stingraykit/collection/GenericCollection.h>
#include <stingraykit/collection/IDictionary.h>
#include <stingraykit/collection/KeyExceptionCreator.h>
#include <stingraykit/function/function.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename MapType_ >
	class GenericDictionary : public virtual IDictionary<typename MapType_::key_type, typename MapType_::mapped_type>
	{
		static_assert(comparers::IsRelationalComparer<typename MapType_::key_compare>::Value, "Expected Relational comparer");

	public:
		using KeyType = typename MapType_::key_type;
		using ValueType = typename MapType_::mapped_type;

		using PairType = KeyValuePair<KeyType, ValueType>;
		using MapType = MapType_;

		using ImplType = GenericCollection<MapType, PairType>;

	private:
		ImplType			_impl;

	public:
		GenericDictionary()
		{ }

		explicit GenericDictionary(const shared_ptr<IEnumerable<PairType>>& enumerable)
			:	GenericDictionary(STINGRAYKIT_REQUIRE_NOT_NULL(enumerable)->GetEnumerator())
		{ }

		explicit GenericDictionary(const shared_ptr<IEnumerator<PairType>>& enumerator)
		{
			STINGRAYKIT_CHECK(enumerator, NullArgumentException("enumerator"));
			FOR_EACH(const PairType pair IN enumerator)
				Set(pair.Key, pair.Value);
		}

		shared_ptr<IEnumerator<PairType>> GetEnumerator() const override
		{ return _impl.GetEnumerator(); }

		shared_ptr<IEnumerable<PairType>> Reverse() const override
		{ return _impl.Reverse(); }

		size_t GetCount() const override
		{ return _impl.GetCount(); }

		bool IsEmpty() const override
		{ return _impl.IsEmpty(); }

		bool ContainsKey(const KeyType& key) const override
		{ return _impl.GetItems().find(key) != _impl.GetItems().end(); }

		shared_ptr<IEnumerator<PairType>> Find(const KeyType& key) const override
		{
			const auto it = _impl.GetItems().find(key);
			if (it == _impl.GetItems().end())
				return MakeEmptyEnumerator();

			return EnumeratorFromStlIterators<PairType>(it, _impl.GetItems().end(), _impl.GetItemsHolder());
		}

		shared_ptr<IEnumerator<PairType>> ReverseFind(const KeyType& key) const override
		{
			auto it = _impl.GetItems().find(key);
			if (it == _impl.GetItems().end())
				return MakeEmptyEnumerator();

			return EnumeratorFromStlIterators<PairType>(typename MapType::const_reverse_iterator(++it), _impl.GetItems().crend(), _impl.GetItemsHolder());
		}

		ValueType Get(const KeyType& key) const override
		{
			const auto it = _impl.GetItems().find(key);
			STINGRAYKIT_CHECK(it != _impl.GetItems().end(), CreateKeyNotFoundException(key));
			return it->second;
		}

		bool TryGet(const KeyType& key, ValueType& outValue) const override
		{
			const auto it = _impl.GetItems().find(key);
			if (it == _impl.GetItems().end())
				return false;

			outValue = it->second;
			return true;
		}

		bool Add(const KeyType& key, const ValueType& value) override
		{ return DoAdd<MapType>(key, value, 0); }

		void Set(const KeyType& key, const ValueType& value) override
		{ DoSet<MapType>(key, value, 0); }

		bool Remove(const KeyType& key) override
		{
			const auto it = _impl.GetItems().find(key);
			if (it == _impl.GetItems().end())
				return false;

			if (_impl.CopyOnWrite())
				_impl.GetItems().erase(key);
			else
				_impl.GetItems().erase(it);

			return true;
		}

		size_t RemoveWhere(const function<bool (const KeyType&, const ValueType&)>& pred) override
		{
			_impl.CopyOnWrite();
			size_t ret = 0;
			for (auto it = _impl.GetItems().begin(); it != _impl.GetItems().end(); )
			{
				if (pred(it->first, it->second))
				{
					it = _impl.GetItems().erase(it);
					++ret;
				}
				else
					++it;
			}
			return ret;
		}

		void Clear() override
		{ _impl.Clear(); }

	private:
		template < typename MapType__ >
		auto DoAdd(const KeyType& key, const ValueType& value, int) -> decltype(std::declval<MapType__>().lower_bound(key), bool())
		{
			const auto it = _impl.GetItems().lower_bound(key);
			if (it != _impl.GetItems().end() && !typename MapType__::key_compare()(key, it->first))
				return false;

			if (_impl.CopyOnWrite())
				_impl.GetItems().emplace(key, value);
			else
				_impl.GetItems().emplace_hint(it, key, value);

			return true;
		}

		template < typename MapType__ >
		bool DoAdd(const KeyType& key, const ValueType& value, long)
		{
			_impl.CopyOnWrite();
			return _impl.GetItems().emplace(key, value).second;
		}

		template < typename MapType__ >
		auto DoSet(const KeyType& key, const ValueType& value, int) -> decltype(std::declval<MapType__>().lower_bound(key), void())
		{
			_impl.CopyOnWrite();
			const auto it = _impl.GetItems().lower_bound(key);
			if (it != _impl.GetItems().end() && !typename MapType__::key_compare()(key, it->first))
				it->second = value;
			else
				_impl.GetItems().emplace_hint(it, key, value);
		}

		template < typename MapType__ >
		void DoSet(const KeyType& key, const ValueType& value, long)
		{
			_impl.CopyOnWrite();
			const auto it = _impl.GetItems().find(key);
			if (it != _impl.GetItems().end())
				it->second = value;
			else
				_impl.GetItems().emplace(key, value);
		}
	};

	/** @} */

}

#endif
