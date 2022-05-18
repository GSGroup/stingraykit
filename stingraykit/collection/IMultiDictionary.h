#ifndef STINGRAYKIT_COLLECTION_IMULTIDICTIONARY_H
#define STINGRAYKIT_COLLECTION_IMULTIDICTIONARY_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/ICollection.h>
#include <stingraykit/collection/KeyValuePair.h>
#include <stingraykit/optional.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename KeyType_, typename ValueType_ >
	struct IReadonlyMultiDictionary
		:	public virtual ICollection<KeyValuePair<KeyType_, ValueType_>>,
			public virtual IReversableEnumerable<KeyValuePair<KeyType_, ValueType_>>
	{
		using KeyType = KeyType_;
		using ValueType = ValueType_;
		using PairType = KeyValuePair<KeyType, ValueType>;

		~IReadonlyMultiDictionary() override { }

		virtual bool ContainsKey(const KeyType& key) const = 0;
		virtual size_t CountKey(const KeyType& key) const = 0;

		virtual shared_ptr<IEnumerator<PairType>> Find(const KeyType& key) const = 0;
		virtual shared_ptr<IEnumerator<PairType>> ReverseFind(const KeyType& key) const = 0;

		virtual ValueType GetFirst(const KeyType& key) const = 0;
		virtual bool TryGetFirst(const KeyType& key, ValueType& outValue) const = 0;

		virtual shared_ptr<IEnumerator<ValueType>> GetAll(const KeyType& key) const = 0;
	};


	template < typename T >
	struct IsInheritedIReadonlyMultiDictionary : public IsInherited2ParamTemplate<T, IReadonlyMultiDictionary>
	{ };


	template < typename KeyType_, typename ValueType_ >
	struct IMultiDictionary
		:	public virtual IReadonlyMultiDictionary<KeyType_, ValueType_>
	{
		using KeyType = KeyType_;
		using ValueType = ValueType_;
		using PairType = KeyValuePair<KeyType, ValueType>;

		~IMultiDictionary() override { }

		virtual void Add(const KeyType& key, const ValueType& value) = 0;

		virtual void RemoveFirst(const KeyType& key, const optional<ValueType>& value = null) = 0;
		virtual bool TryRemoveFirst(const KeyType& key, const optional<ValueType>& value = null) = 0;

		virtual size_t RemoveAll(const KeyType& key) = 0;

		virtual size_t RemoveWhere(const function<bool (const KeyType&, const ValueType&)>& pred) = 0;

		virtual void Clear() = 0;
	};


	template < typename T >
	struct IsInheritedIMultiDictionary : public IsInherited2ParamTemplate<T, IMultiDictionary>
	{ };

	/** @} */

}

#endif
