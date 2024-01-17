#ifndef STINGRAYKIT_COLLECTION_KEYVALUEENUMERABLEHELPERS_H
#define STINGRAYKIT_COLLECTION_KEYVALUEENUMERABLEHELPERS_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableWrapper.h>
#include <stingraykit/GetMember.h>

namespace stingray
{

	template<typename EnumeratorType>
	shared_ptr<IEnumerator<typename EnumeratorType::ItemType::KeyType> > KeysEnumerator(const shared_ptr<EnumeratorType>& enumerator)
	{
		typedef typename EnumeratorType::ItemType PairType;
		return WrapEnumerator(enumerator, GetMember(&PairType::Key));
	}

	template<typename EnumerableType>
	shared_ptr<IEnumerable<typename EnumerableType::ItemType::KeyType> > KeysEnumerable(const shared_ptr<EnumerableType>& enumerable)
	{
		typedef typename EnumerableType::ItemType PairType;
		return WrapEnumerable(enumerable, GetMember(&PairType::Key));
	}

	template<typename EnumeratorType>
	shared_ptr<IEnumerator<typename EnumeratorType::ItemType::ValueType> > ValuesEnumerator(const shared_ptr<EnumeratorType>& enumerator)
	{
		typedef typename EnumeratorType::ItemType PairType;
		return WrapEnumerator(enumerator, GetMember(&PairType::Value));
	}

	template<typename EnumerableType>
	shared_ptr<IEnumerable<typename EnumerableType::ItemType::ValueType> > ValuesEnumerable(const shared_ptr<EnumerableType>& enumerable)
	{
		typedef typename EnumerableType::ItemType PairType;
		return WrapEnumerable(enumerable, GetMember(&PairType::Value));
	}

}

#endif
