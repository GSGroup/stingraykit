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
	class GenericList : public virtual IList<typename VectorType_::value_type>
	{
	public:
		using ValueType = typename VectorType_::value_type;

	private:
		using VectorType = VectorType_;

		using ImplType = GenericCollection<VectorType>;

	private:
		ImplType			_impl;

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

		shared_ptr<IEnumerator<ValueType>> GetEnumerator() const override
		{ return _impl.GetEnumerator(); }

		shared_ptr<IEnumerable<ValueType>> Reverse() const override
		{ return _impl.Reverse(); }

		size_t GetCount() const override
		{ return _impl.GetCount(); }

		bool IsEmpty() const override
		{ return _impl.IsEmpty(); }

		bool Contains(const ValueType& value) const override
		{
			const VectorType& items = _impl.GetItems();
			return std::find(items.begin(), items.end(), value) != items.end();
		}

		optional<size_t> IndexOf(const ValueType& value) const override
		{
			const VectorType& items = _impl.GetItems();

			const auto it = std::find(items.begin(), items.end(), value);
			return it == items.end() ? null : make_optional_value(std::distance(items.begin(), it));
		}

		ValueType Get(size_t index) const override
		{
			const VectorType& items = _impl.GetItems();

			STINGRAYKIT_CHECK(index < items.size(), IndexOutOfRangeException(index, items.size()));
			return items[index];
		}

		bool TryGet(size_t index, ValueType& value) const override
		{
			const VectorType& items = _impl.GetItems();

			if (index >= items.size())
				return false;

			value = items[index];
			return true;
		}

		void Add(const ValueType& value) override
		{
			_impl.CopyOnWrite();
			_impl.GetItems().push_back(value);
		}

		void Set(size_t index, const ValueType& value) override
		{
			const VectorType& items = _impl.GetItems();
			STINGRAYKIT_CHECK(index < items.size(), IndexOutOfRangeException(index, items.size()));

			_impl.CopyOnWrite();
			_impl.GetItems()[index] = value;
		}

		void Insert(size_t index, const ValueType& value) override
		{
			const VectorType& items = _impl.GetItems();
			STINGRAYKIT_CHECK(index <= items.size(), IndexOutOfRangeException(index, items.size()));

			_impl.CopyOnWrite();

			VectorType& newItems = _impl.GetItems();
			newItems.insert(std::next(newItems.begin(), index), value);
		}

		void RemoveAt(size_t index) override
		{
			const VectorType& items = _impl.GetItems();
			STINGRAYKIT_CHECK(index < items.size(), IndexOutOfRangeException(index, items.size()));

			_impl.CopyOnWrite();

			VectorType& newItems = _impl.GetItems();
			newItems.erase(std::next(newItems.begin(), index));
		}

		bool Remove(const ValueType& value) override
		{
			const VectorType& items = _impl.GetItems();
			const auto it = std::find(items.begin(), items.end(), value);
			if (it == items.end())
				return false;

			const size_t index = std::distance(items.begin(), it);

			_impl.CopyOnWrite();

			VectorType& newItems = _impl.GetItems();
			newItems.erase(std::next(newItems.begin(), index));
			return true;
		}

		size_t RemoveAll(const function<bool (const ValueType&)>& pred) override
		{
			_impl.CopyOnWrite();

			VectorType& items = _impl.GetItems();

			const auto it = std::remove_if(items.begin(), items.end(), pred);
			const size_t ret = std::distance(it, items.end());

			items.erase(it, items.end());
			return ret;
		}

		void Clear() override
		{ _impl.Clear(); }
	};

	/** @} */

}

#endif
