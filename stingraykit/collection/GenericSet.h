#ifndef STINGRAYKIT_COLLECTION_GENERICSET_H
#define STINGRAYKIT_COLLECTION_GENERICSET_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/GenericCollection.h>
#include <stingraykit/collection/ISet.h>
#include <stingraykit/function/function.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename SetType_ >
	class GenericSet
		:	public virtual ISet<typename SetType_::value_type>,
			public GenericCollection<SetType_>
	{
		static_assert(comparers::IsRelationalComparer<typename SetType_::value_compare>::Value, "Expected Relational comparer");

	public:
		using ValueType = typename SetType_::value_type;

	private:
		using SetType = SetType_;

		using BaseType = GenericCollection<SetType>;

	public:
		GenericSet()
		{ }

		explicit GenericSet(const shared_ptr<IEnumerable<ValueType>>& enumerable)
			:	GenericSet(STINGRAYKIT_REQUIRE_NOT_NULL(enumerable)->GetEnumerator())
		{ }

		explicit GenericSet(const shared_ptr<IEnumerator<ValueType>>& enumerator)
		{
			STINGRAYKIT_CHECK(enumerator, NullArgumentException("enumerator"));
			Enumerable::ForEach(enumerator, Bind(&GenericSet::Add, this, _1));
		}

		bool Contains(const ValueType& value) const override
		{ return BaseType::_items->find(value) != BaseType::_items->end(); }

		shared_ptr<IEnumerator<ValueType>> Find(const ValueType& value) const override
		{
			const auto it = BaseType::_items->find(value);
			if (it == BaseType::_items->end())
				return MakeEmptyEnumerator();

			return EnumeratorFromStlIterators(it, BaseType::_items->end(), BaseType::GetItemsHolder());
		}

		shared_ptr<IEnumerator<ValueType>> ReverseFind(const ValueType& value) const override
		{
			auto it = BaseType::_items->find(value);
			if (it == BaseType::_items->end())
				return MakeEmptyEnumerator();

			return EnumeratorFromStlIterators(typename SetType::const_reverse_iterator(++it), BaseType::_items->crend(), BaseType::GetItemsHolder());
		}

		bool Add(const ValueType& value) override
		{ return DoAdd<SetType>(value, 0); }

		bool Remove(const ValueType& value) override
		{
			const auto it = BaseType::_items->find(value);
			if (it == BaseType::_items->end())
				return false;

			if (BaseType::CopyOnWrite())
				BaseType::_items->erase(value);
			else
				BaseType::_items->erase(it);

			return true;
		}

		size_t RemoveWhere(const function<bool (const ValueType&)>& pred) override
		{
			BaseType::CopyOnWrite();
			size_t ret = 0;
			for (auto it = BaseType::_items->begin(); it != BaseType::_items->end(); )
			{
				if (pred(*it))
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
		template < typename SetType__ >
		auto DoAdd(const ValueType& value, int) -> decltype(std::declval<SetType__>().lower_bound(value), bool())
		{
			const auto it = BaseType::_items->lower_bound(value);
			if (it != BaseType::_items->end() && !typename SetType__::value_compare()(value, *it))
				return false;

			if (BaseType::CopyOnWrite())
				BaseType::_items->insert(value);
			else
				BaseType::_items->insert(it, value);

			return true;
		}

		template < typename SetType__ >
		bool DoAdd(const ValueType& value, long)
		{
			BaseType::CopyOnWrite();
			return BaseType::_items->insert(value).second;
		}
	};

	/** @} */

}

#endif
