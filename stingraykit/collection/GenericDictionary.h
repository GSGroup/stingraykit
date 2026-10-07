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
	class GenericDictionary
		:	public virtual IDictionary<typename MapType_::key_type, typename MapType_::mapped_type>,
			public GenericCollection<MapType_, KeyValuePair<typename MapType_::key_type, typename MapType_::mapped_type>>
	{
		static_assert(comparers::IsRelationalComparer<typename MapType_::key_compare>::Value, "Expected Relational comparer");

	public:
		using KeyType = typename MapType_::key_type;
		using ValueType = typename MapType_::mapped_type;

		using PairType = KeyValuePair<KeyType, ValueType>;
		using MapType = MapType_;

		using BaseType = GenericCollection<MapType, PairType>;

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

		bool ContainsKey(const KeyType& key) const override
		{ return BaseType::_items->find(key) != BaseType::_items->end(); }

		shared_ptr<IEnumerator<PairType>> Find(const KeyType& key) const override
		{
			const auto it = BaseType::_items->find(key);
			if (it == BaseType::_items->end())
				return MakeEmptyEnumerator();

			return EnumeratorFromStlIterators<PairType>(it, BaseType::_items->end(), BaseType::GetItemsHolder());
		}

		shared_ptr<IEnumerator<PairType>> ReverseFind(const KeyType& key) const override
		{
			auto it = BaseType::_items->find(key);
			if (it == BaseType::_items->end())
				return MakeEmptyEnumerator();

			return EnumeratorFromStlIterators<PairType>(typename MapType::const_reverse_iterator(++it), BaseType::_items->crend(), BaseType::GetItemsHolder());
		}

		ValueType Get(const KeyType& key) const override
		{
			const auto it = BaseType::_items->find(key);
			STINGRAYKIT_CHECK(it != BaseType::_items->end(), CreateKeyNotFoundException(key));
			return it->second;
		}

		bool TryGet(const KeyType& key, ValueType& outValue) const override
		{
			const auto it = BaseType::_items->find(key);
			if (it != BaseType::_items->end())
			{
				outValue = it->second;
				return true;
			}
			else
				return false;
		}

		bool Add(const KeyType& key, const ValueType& value) override
		{ return DoAdd<MapType>(key, value, 0); }

		void Set(const KeyType& key, const ValueType& value) override
		{ DoSet<MapType>(key, value, 0); }

		bool Remove(const KeyType& key) override
		{
			const auto it = BaseType::_items->find(key);
			if (it == BaseType::_items->end())
				return false;

			if (BaseType::CopyOnWrite())
				BaseType::_items->erase(key);
			else
				BaseType::_items->erase(it);

			return true;
		}

		size_t RemoveWhere(const function<bool (const KeyType&, const ValueType&)>& pred) override
		{
			BaseType::CopyOnWrite();
			size_t ret = 0;
			for (auto it = BaseType::_items->begin(); it != BaseType::_items->end(); )
			{
				if (pred(it->first, it->second))
				{
					it = BaseType::_items->erase(it);
					++ret;
				}
				else
					++it;
			}
			return ret;
		}

		void Clear() override
		{ BaseType::DoClear(); }

	private:
		template < typename MapType__ >
		auto DoAdd(const KeyType& key, const ValueType& value, int) -> decltype(std::declval<MapType__>().lower_bound(key), bool())
		{
			const auto it = BaseType::_items->lower_bound(key);
			if (it != BaseType::_items->end() && !typename MapType__::key_compare()(key, it->first))
				return false;

			if (BaseType::CopyOnWrite())
				BaseType::_items->emplace(key, value);
			else
				BaseType::_items->emplace_hint(it, key, value);

			return true;
		}

		template < typename MapType__ >
		bool DoAdd(const KeyType& key, const ValueType& value, long)
		{
			BaseType::CopyOnWrite();
			return BaseType::_items->emplace(key, value).second;
		}

		template < typename MapType__ >
		auto DoSet(const KeyType& key, const ValueType& value, int) -> decltype(std::declval<MapType__>().lower_bound(key), void())
		{
			BaseType::CopyOnWrite();
			const auto it = BaseType::_items->lower_bound(key);
			if (it != BaseType::_items->end() && !typename MapType__::key_compare()(key, it->first))
				it->second = value;
			else
				BaseType::_items->emplace_hint(it, key, value);
		}

		template < typename MapType__ >
		void DoSet(const KeyType& key, const ValueType& value, long)
		{
			BaseType::CopyOnWrite();
			const auto it = BaseType::_items->find(key);
			if (it != BaseType::_items->end())
				it->second = value;
			else
				BaseType::_items->emplace(key, value);
		}
	};

	/** @} */

}

#endif
