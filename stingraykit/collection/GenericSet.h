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
	class GenericSet : public virtual ISet<typename SetType_::value_type>
	{
		static_assert(comparers::IsRelationalComparer<typename SetType_::value_compare>::Value, "Expected Relational comparer");

	public:
		using ValueType = typename SetType_::value_type;

	private:
		using SetType = SetType_;

		using ImplType = GenericCollection<SetType>;

	private:
		ImplType			_impl;

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

		shared_ptr<IEnumerator<ValueType>> GetEnumerator() const override
		{ return _impl.GetEnumerator(); }

		shared_ptr<IEnumerable<ValueType>> Reverse() const override
		{ return _impl.Reverse(); }

		size_t GetCount() const override
		{ return _impl.GetCount(); }

		bool IsEmpty() const override
		{ return _impl.IsEmpty(); }

		bool Contains(const ValueType& value) const override
		{ return _impl.GetItems().find(value) != _impl.GetItems().end(); }

		shared_ptr<IEnumerator<ValueType>> Find(const ValueType& value) const override
		{
			const auto it = _impl.GetItems().find(value);
			if (it == _impl.GetItems().end())
				return MakeEmptyEnumerator();

			return EnumeratorFromStlIterators(it, _impl.GetItems().end(), _impl.GetItemsHolder());
		}

		shared_ptr<IEnumerator<ValueType>> ReverseFind(const ValueType& value) const override
		{
			auto it = _impl.GetItems().find(value);
			if (it == _impl.GetItems().end())
				return MakeEmptyEnumerator();

			return EnumeratorFromStlIterators(typename SetType::const_reverse_iterator(++it), _impl.GetItems().crend(), _impl.GetItemsHolder());
		}

		bool Add(const ValueType& value) override
		{ return DoAdd<SetType>(value, 0); }

		bool Remove(const ValueType& value) override
		{
			const auto it = _impl.GetItems().find(value);
			if (it == _impl.GetItems().end())
				return false;

			if (_impl.CopyOnWrite())
				_impl.GetItems().erase(value);
			else
				_impl.GetItems().erase(it);

			return true;
		}

		size_t RemoveWhere(const function<bool (const ValueType&)>& pred) override
		{
			_impl.CopyOnWrite();
			size_t ret = 0;
			for (auto it = _impl.GetItems().begin(); it != _impl.GetItems().end(); )
			{
				if (pred(*it))
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
		template < typename SetType__ >
		auto DoAdd(const ValueType& value, int) -> decltype(std::declval<SetType__>().lower_bound(value), bool())
		{
			const auto it = _impl.GetItems().lower_bound(value);
			if (it != _impl.GetItems().end() && !typename SetType__::value_compare()(value, *it))
				return false;

			if (_impl.CopyOnWrite())
				_impl.GetItems().insert(value);
			else
				_impl.GetItems().insert(it, value);

			return true;
		}

		template < typename SetType__ >
		bool DoAdd(const ValueType& value, long)
		{
			_impl.CopyOnWrite();
			return _impl.GetItems().insert(value).second;
		}
	};

	/** @} */

}

#endif
