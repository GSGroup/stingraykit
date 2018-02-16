#ifndef STINGRAYKIT_COLLECTION_IDICTIONARY_H
#define STINGRAYKIT_COLLECTION_IDICTIONARY_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
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
	struct IReadonlyDictionary :
		public virtual ICollection<KeyValuePair<KeyType_, ValueType_> >,
		public virtual IReversableEnumerable<KeyValuePair<KeyType_, ValueType_> >
	{
		typedef KeyType_							KeyType;
		typedef ValueType_							ValueType;
		typedef KeyValuePair<KeyType, ValueType>	PairType;

		virtual ~IReadonlyDictionary() { }

		virtual bool ContainsKey(const KeyType& key) const = 0;

		virtual shared_ptr<IEnumerator<PairType> > Find(const KeyType& key) const = 0;
		virtual shared_ptr<IEnumerator<PairType> > ReverseFind(const KeyType& key) const = 0;

		virtual ValueType Get(const KeyType& key) const = 0;
		virtual bool TryGet(const KeyType& key, ValueType& outValue) const = 0;
	};


	template < typename T >
	struct IsInheritedIReadonlyDictionary : public IsInherited2ParamTemplate<T, IReadonlyDictionary>
	{ };


	template < typename KeyType_, typename ValueType_ >
	struct IDictionary :
		public virtual IReadonlyDictionary<KeyType_, ValueType_>
	{
		typedef KeyType_							KeyType;
		typedef ValueType_							ValueType;
		typedef KeyValuePair<KeyType, ValueType>	PairType;

		virtual ~IDictionary() { }

		virtual void Set(const KeyType& key, const ValueType& value) = 0;

		virtual void Remove(const KeyType& key) = 0;
		virtual bool TryRemove(const KeyType& key) = 0;

		virtual size_t RemoveWhere(const function<bool (const KeyType&, const ValueType&)>& pred) = 0;

		virtual void Clear() = 0;
	};


	template < typename T >
	struct IsInheritedIDictionary : public IsInherited2ParamTemplate<T, IDictionary>
	{ };


	template<typename EnumeratorType>
	shared_ptr<IEnumerator<typename EnumeratorType::ItemType::KeyType> > KeysEnumerator(const shared_ptr<EnumeratorType>& enumerator)
	{
		typedef typename EnumeratorType::ItemType PairType;
		return WrapEnumerator(enumerator, &PairType::GetKey);
	}

	template<typename EnumerableType>
	shared_ptr<IEnumerable<typename EnumerableType::ItemType::KeyType> > KeysEnumerable(const shared_ptr<EnumerableType>& enumerable)
	{
		typedef typename EnumerableType::ItemType PairType;
		return WrapEnumerable(enumerable, &PairType::GetKey);
	}

	template<typename EnumeratorType>
	shared_ptr<IEnumerator<typename EnumeratorType::ItemType::ValueType> > ValuesEnumerator(const shared_ptr<EnumeratorType>& enumerator)
	{
		typedef typename EnumeratorType::ItemType PairType;
		return WrapEnumerator(enumerator, &PairType::GetValue);
	}

	template<typename EnumerableType>
	shared_ptr<IEnumerable<typename EnumerableType::ItemType::ValueType> > ValuesEnumerable(const shared_ptr<EnumerableType>& enumerable)
	{
		typedef typename EnumerableType::ItemType PairType;
		return WrapEnumerable(enumerable, &PairType::GetValue);
	}

	/** @} */

}

#endif
