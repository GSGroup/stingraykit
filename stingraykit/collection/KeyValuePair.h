#ifndef STINGRAYKIT_COLLECTION_KEYVALUEPAIR_H
#define STINGRAYKIT_COLLECTION_KEYVALUEPAIR_H

// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
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
		typedef KeyType_	KeyType;
		typedef ValueType_	ValueType;

		KeyType		Key;
		ValueType	Value;

		KeyValuePair()
			: Key(), Value()
		{ }

		KeyValuePair(const KeyType& key, const ValueType& value)
			: Key(key), Value(value)
		{ }

		KeyValuePair(const std::pair<KeyType, ValueType>& pair)
			: Key(pair.first), Value(pair.second)
		{ }

		int Compare(const KeyValuePair& other) const
		{ return CompareMembersCmp(&KeyValuePair::Key, &KeyValuePair::Value)(*this, other); }

		std::string ToString() const	{ return StringBuilder() % Key % " -> " % Value; }
	};

	/** @} */

}

#endif
