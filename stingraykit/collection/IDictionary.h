#ifndef STINGRAYKIT_COLLECTION_IDICTIONARY_H
#define STINGRAYKIT_COLLECTION_IDICTIONARY_H

// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/ICollection.h>
#include <stingraykit/collection/KeyValuePair.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename KeyType_, typename ValueType_ >
	struct IReadonlyDictionary
		:	public virtual ICollection<KeyValuePair<KeyType_, ValueType_>>,
			public virtual IReversableEnumerable<KeyValuePair<KeyType_, ValueType_>>
	{
		using KeyType = KeyType_;
		using ValueType = ValueType_;
		using PairType = KeyValuePair<KeyType, ValueType>;

		~IReadonlyDictionary() override { }

		virtual bool ContainsKey(const KeyType& key) const = 0;

		virtual shared_ptr<IEnumerator<PairType>> Find(const KeyType& key) const = 0;
		virtual shared_ptr<IEnumerator<PairType>> ReverseFind(const KeyType& key) const = 0;

		virtual ValueType Get(const KeyType& key) const = 0;
		virtual bool TryGet(const KeyType& key, ValueType& outValue) const = 0;
	};


	template < typename T >
	struct IsInheritedIReadonlyDictionary : public IsInherited2ParamTemplate<T, IReadonlyDictionary>
	{ };


	template < typename KeyType_, typename ValueType_ >
	struct IDictionary
		:	public virtual IReadonlyDictionary<KeyType_, ValueType_>
	{
		using KeyType = KeyType_;
		using ValueType = ValueType_;
		using PairType = KeyValuePair<KeyType, ValueType>;

		~IDictionary() override { }

		virtual bool Add(const KeyType& key, const ValueType& value) = 0;
		virtual void Set(const KeyType& key, const ValueType& value) = 0;

		virtual bool Remove(const KeyType& key) = 0;
		virtual size_t RemoveWhere(const function<bool (const KeyType&, const ValueType&)>& pred) = 0;

		virtual void Clear() = 0;
	};


	template < typename T >
	struct IsInheritedIDictionary : public IsInherited2ParamTemplate<T, IDictionary>
	{ };

	/** @} */

}

#endif
