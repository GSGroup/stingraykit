#ifndef STINGRAYKIT_COLLECTION_KEYVALUEPAIR_H
#define STINGRAYKIT_COLLECTION_KEYVALUEPAIR_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/compare/MemberListComparer.h>
#include <stingraykit/string/ToString.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename KeyType_, typename ValueType_ >
	struct KeyValuePair
	{
		STINGRAYKIT_DEFAULTCOPYABLE(KeyValuePair);
		STINGRAYKIT_DEFAULTMOVABLE(KeyValuePair);

	public:
		using KeyType = KeyType_;
		using ValueType = ValueType_;

	public:
		KeyType		Key;
		ValueType	Value;

	public:
		template <
				typename KeyType__ = KeyType,
				typename ValueType__ = ValueType,
				typename EnableIf<IsDefaultConstructible<KeyType__>::Value && IsDefaultConstructible<ValueType__>::Value, int>::ValueT = 0
		>
		KeyValuePair()
			: Key(), Value()
		{ }

		template <
				typename KeyType__ = KeyType,
				typename ValueType__ = ValueType,
				typename EnableIf<IsConstructible<KeyType, const KeyType__&>::Value && IsConstructible<ValueType, const ValueType__&>::Value, int>::ValueT = 0
		>
		KeyValuePair(const KeyType__& key, const ValueType__& value)
			: Key(key), Value(value)
		{ }

		template <
				typename KeyType__ = KeyType,
				typename ValueType__ = ValueType,
				typename EnableIf<IsConstructible<KeyType, KeyType__>::Value && IsConstructible<ValueType, ValueType__>::Value, int>::ValueT = 0
		>
		KeyValuePair(KeyType__&& key, ValueType__&& value)
			: Key(std::forward<KeyType__>(key)), Value(std::forward<ValueType__>(value))
		{ }

		template <
				typename KeyType__,
				typename ValueType__,
				typename EnableIf<IsConstructible<KeyType, const KeyType__&>::Value && IsConstructible<ValueType, const ValueType__&>::Value, int>::ValueT = 0,
				typename EnableIf<IsConvertible<const KeyType__&, KeyType>::Value && IsConvertible<const ValueType__&, ValueType>::Value, int>::ValueT = 0
		>
		KeyValuePair(const KeyValuePair<KeyType__, ValueType__>& pair)
			: Key(pair.Key), Value(pair.Value)
		{ }

		template <
				typename KeyType__,
				typename ValueType__,
				typename EnableIf<IsConstructible<KeyType, const KeyType__&>::Value && IsConstructible<ValueType, const ValueType__&>::Value, int>::ValueT = 0,
				typename EnableIf<!IsConvertible<const KeyType__&, KeyType>::Value || !IsConvertible<const ValueType__&, ValueType>::Value, int>::ValueT = 0
		>
		explicit KeyValuePair(const KeyValuePair<KeyType__, ValueType__>& pair)
			: Key(pair.Key), Value(pair.Value)
		{ }

		template <
				typename KeyType__,
				typename ValueType__,
				typename EnableIf<IsConstructible<KeyType, KeyType__>::Value && IsConstructible<ValueType, ValueType__>::Value, int>::ValueT = 0,
				typename EnableIf<IsConvertible<KeyType__, KeyType>::Value && IsConvertible<ValueType__, ValueType>::Value, int>::ValueT = 0
		>
		KeyValuePair(KeyValuePair<KeyType__, ValueType__>&& pair)
			: Key(std::forward<KeyType__>(pair.Key)), Value(std::forward<ValueType__>(pair.Value))
		{ }

		template <
				typename KeyType__,
				typename ValueType__,
				typename EnableIf<IsConstructible<KeyType, KeyType__>::Value && IsConstructible<ValueType, ValueType__>::Value, int>::ValueT = 0,
				typename EnableIf<!IsConvertible<KeyType__, KeyType>::Value || !IsConvertible<ValueType__, ValueType>::Value, int>::ValueT = 0
		>
		explicit KeyValuePair(KeyValuePair<KeyType__, ValueType__>&& pair)
			: Key(std::forward<KeyType__>(pair.Key)), Value(std::forward<ValueType__>(pair.Value))
		{ }

		template <
				typename KeyType__ = KeyType,
				typename ValueType__ = ValueType,
				typename EnableIf<IsConstructible<KeyType, const KeyType__&>::Value && IsConstructible<ValueType, const ValueType__&>::Value, int>::ValueT = 0,
				typename EnableIf<IsConvertible<const KeyType__&, KeyType>::Value && IsConvertible<const ValueType__&, ValueType>::Value, int>::ValueT = 0
		>
		KeyValuePair(const std::pair<KeyType__, ValueType__>& pair)
			: Key(pair.first), Value(pair.second)
		{ }

		template <
				typename KeyType__ = KeyType,
				typename ValueType__ = ValueType,
				typename EnableIf<IsConstructible<KeyType, const KeyType__&>::Value && IsConstructible<ValueType, const ValueType__&>::Value, int>::ValueT = 0,
				typename EnableIf<!IsConvertible<const KeyType__&, KeyType>::Value || !IsConvertible<const ValueType__&, ValueType>::Value, int>::ValueT = 0
		>
		explicit KeyValuePair(const std::pair<KeyType__, ValueType__>& pair)
			: Key(pair.first), Value(pair.second)
		{ }

		template <
				typename KeyType__ = KeyType,
				typename ValueType__ = ValueType,
				typename EnableIf<IsConstructible<KeyType, KeyType__>::Value && IsConstructible<ValueType, ValueType__>::Value, int>::ValueT = 0,
				typename EnableIf<IsConvertible<KeyType__, KeyType>::Value && IsConvertible<ValueType__, ValueType>::Value, int>::ValueT = 0
		>
		KeyValuePair(std::pair<KeyType__, ValueType__>&& pair)
			: Key(std::forward<KeyType__>(pair.first)), Value(std::forward<ValueType__>(pair.second))
		{ }

		template <
				typename KeyType__ = KeyType,
				typename ValueType__ = ValueType,
				typename EnableIf<IsConstructible<KeyType, KeyType__>::Value && IsConstructible<ValueType, ValueType__>::Value, int>::ValueT = 0,
				typename EnableIf<!IsConvertible<KeyType__, KeyType>::Value || !IsConvertible<ValueType__, ValueType>::Value, int>::ValueT = 0
		>
		explicit KeyValuePair(std::pair<KeyType__, ValueType__>&& pair)
			: Key(std::forward<KeyType__>(pair.first)), Value(std::forward<ValueType__>(pair.second))
		{ }

		int Compare(const KeyValuePair& other) const
		{ return CompareMembersCmp(&KeyValuePair::Key, &KeyValuePair::Value)(*this, other); }

		std::string ToString() const
		{ return StringBuilder() % Key % " -> " % Value; }
	};


	template < typename KeyType_, typename ValueType_ >
	KeyValuePair<typename Decay<KeyType_>::ValueT, typename Decay<ValueType_>::ValueT> MakeKeyValuePair(KeyType_&& key, ValueType_&& value)
	{ return KeyValuePair<typename Decay<KeyType_>::ValueT, typename Decay<ValueType_>::ValueT>(std::forward<KeyType_>(key), std::forward<ValueType_>(value)); }

	/** @} */

}

#endif
