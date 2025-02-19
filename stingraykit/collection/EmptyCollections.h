#ifndef STINGRAYKIT_COLLECTION_EMPTYCOLLECTIONS_H
#define STINGRAYKIT_COLLECTION_EMPTYCOLLECTIONS_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/IDictionary.h>
#include <stingraykit/collection/IList.h>
#include <stingraykit/collection/ISet.h>
#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/KeyExceptionCreator.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	namespace Detail
	{

		template < typename T >
		struct EmptyList final : public virtual IReadonlyList<T>
		{
			using ValueType = typename IReadonlyList<T>::ValueType;

			shared_ptr<IEnumerator<ValueType>> GetEnumerator() const override
			{ return MakeEmptyEnumerator(); }

			shared_ptr<IEnumerable<ValueType>> Reverse() const override
			{ return MakeEmptyEnumerable(); }

			size_t GetCount() const override
			{ return 0; }

			bool IsEmpty() const override
			{ return true; }

			bool Contains(const ValueType& value) const override
			{ return false; }

			optional<size_t> IndexOf(const ValueType& value) const override
			{ return null; }

			ValueType Get(size_t index) const override
			{ STINGRAYKIT_THROW(IndexOutOfRangeException(0, 0)); }

			bool TryGet(size_t index, ValueType& value) const override
			{ return false; }
		};

		template < typename T >
		struct EmptySet final : public virtual IReadonlySet<T>
		{
			using ValueType = typename IReadonlySet<T>::ValueType;

			shared_ptr<IEnumerator<ValueType>> GetEnumerator() const override
			{ return MakeEmptyEnumerator(); }

			shared_ptr<IEnumerable<ValueType>> Reverse() const override
			{ return MakeEmptyEnumerable(); }

			size_t GetCount() const override
			{ return 0; }

			bool IsEmpty() const override
			{ return true; }

			bool Contains(const ValueType& value) const override
			{ return false; }

			shared_ptr<IEnumerator<ValueType>> Find(const ValueType& value) const override
			{ return MakeEmptyEnumerator(); }

			shared_ptr<IEnumerator<ValueType>> ReverseFind(const ValueType& value) const override
			{ return MakeEmptyEnumerator(); }
		};

		template < typename KeyType_, typename ValueType_ >
		struct EmptyDictionary final : public virtual IReadonlyDictionary<KeyType_, ValueType_>
		{
			using base = IReadonlyDictionary<KeyType_, ValueType_>;

			using KeyType = typename base::KeyType;
			using ValueType = typename base::ValueType;
			using PairType = typename base::PairType;

			shared_ptr<IEnumerator<PairType>> GetEnumerator() const override
			{ return MakeEmptyEnumerator(); }

			shared_ptr<IEnumerable<PairType>> Reverse() const override
			{ return MakeEmptyEnumerable(); }

			size_t GetCount() const override
			{ return 0; }

			bool IsEmpty() const override
			{ return true; }

			bool ContainsKey(const KeyType& key) const override
			{ return false; }

			shared_ptr<IEnumerator<PairType>> Find(const KeyType& key) const override
			{ return MakeEmptyEnumerator(); }

			shared_ptr<IEnumerator<PairType>> ReverseFind(const KeyType& key) const override
			{ return MakeEmptyEnumerator(); }

			ValueType Get(const KeyType& key) const override
			{ STINGRAYKIT_THROW(CreateKeyNotFoundException(key)); }

			bool TryGet(const KeyType& key, ValueType& outValue) const override
			{ return false; }
		};

		struct EmptyCollectionProxy
		{
			template < typename T >
			operator shared_ptr<IReadonlyList<T>> () const
			{ return make_shared_ptr<EmptyList<T>>(); }

			template < typename T >
			operator shared_ptr<IReadonlySet<T>> () const
			{ return make_shared_ptr<EmptySet<T>>(); }

			template < typename KeyType_, typename ValueType_ >
			operator shared_ptr<IReadonlyDictionary<KeyType_, ValueType_>> () const
			{ return make_shared_ptr<EmptyDictionary<KeyType_, ValueType_>>(); }
		};

	}


	inline Detail::EmptyCollectionProxy MakeEmptyCollection()
	{ return Detail::EmptyCollectionProxy(); }

	/** @} */

}

#endif
