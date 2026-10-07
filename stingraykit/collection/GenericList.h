#ifndef STINGRAYKIT_COLLECTION_GENERICLIST_H
#define STINGRAYKIT_COLLECTION_GENERICLIST_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/GenericCollection.h>
#include <stingraykit/collection/IList.h>
#include <stingraykit/function/function.h>

#include <algorithm>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename VectorType_ >
	class GenericList
		:	public virtual IList<typename VectorType_::value_type>,
			public GenericCollection<VectorType_>
	{
	public:
		using ValueType = typename VectorType_::value_type;

	private:
		using VectorType = VectorType_;

		using BaseType = GenericCollection<VectorType>;

	public:
		GenericList()
		{ }

		explicit GenericList(const shared_ptr<IEnumerable<ValueType>>& enumerable)
			:	GenericList(STINGRAYKIT_REQUIRE_NOT_NULL(enumerable)->GetEnumerator())
		{ }

		explicit GenericList(const shared_ptr<IEnumerator<ValueType>>& enumerator)
		{
			STINGRAYKIT_CHECK(enumerator, NullArgumentException("enumerator"));
			Enumerable::ForEach(enumerator, Bind(&GenericList::Add, this, _1));
		}

		bool Contains(const ValueType& value) const override
		{ return std::find(BaseType::_items->begin(), BaseType::_items->end(), value) != BaseType::_items->end(); }

		optional<size_t> IndexOf(const ValueType& value) const override
		{
			const auto it = std::find(BaseType::_items->begin(), BaseType::_items->end(), value);
			return it == BaseType::_items->end() ? null : make_optional_value(std::distance(BaseType::_items->begin(), it));
		}

		ValueType Get(size_t index) const override
		{
			STINGRAYKIT_CHECK(index < BaseType::_items->size(), IndexOutOfRangeException(index, BaseType::_items->size()));
			return (*BaseType::_items)[index];
		}

		bool TryGet(size_t index, ValueType& value) const override
		{
			if (index >= BaseType::_items->size())
				return false;

			value = (*BaseType::_items)[index];
			return true;
		}

		void Add(const ValueType& value) override
		{
			BaseType::CopyOnWrite();
			BaseType::_items->push_back(value);
		}

		void Set(size_t index, const ValueType& value) override
		{
			STINGRAYKIT_CHECK(index < BaseType::_items->size(), IndexOutOfRangeException(index, BaseType::_items->size()));
			BaseType::CopyOnWrite();
			(*BaseType::_items)[index] = value;
		}

		void Insert(size_t index, const ValueType& value) override
		{
			STINGRAYKIT_CHECK(index <= BaseType::_items->size(), IndexOutOfRangeException(index, BaseType::_items->size()));
			BaseType::CopyOnWrite();
			BaseType::_items->insert(std::next(BaseType::_items->begin(), index), value);
		}

		void RemoveAt(size_t index) override
		{
			STINGRAYKIT_CHECK(index < BaseType::_items->size(), IndexOutOfRangeException(index, BaseType::_items->size()));
			BaseType::CopyOnWrite();
			BaseType::_items->erase(std::next(BaseType::_items->begin(), index));
		}

		bool Remove(const ValueType& value) override
		{
			const auto it = std::find(BaseType::_items->begin(), BaseType::_items->end(), value);
			if (it == BaseType::_items->end())
				return false;

			const size_t index = std::distance(BaseType::_items->begin(), it);

			BaseType::CopyOnWrite();
			BaseType::_items->erase(std::next(BaseType::_items->begin(), index));
			return true;
		}

		size_t RemoveAll(const function<bool (const ValueType&)>& pred) override
		{
			BaseType::CopyOnWrite();
			const auto it = std::remove_if(BaseType::_items->begin(), BaseType::_items->end(), pred);
			const size_t ret = std::distance(it, BaseType::_items->end());
			BaseType::_items->erase(it, BaseType::_items->end());
			return ret;
		}

		void Clear() override
		{ BaseType::DoClear(); }
	};

	/** @} */

}

#endif
