#ifndef STINGRAYKIT_COLLECTION_KEYNOTFOUNDEXCEPTIONCREATOR_H
#define STINGRAYKIT_COLLECTION_KEYNOTFOUNDEXCEPTIONCREATOR_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/string/ToString.h>
#include <stingraykit/exception.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	namespace Detail
	{
		template < typename KeyType, bool StringRepresentable = IsStringRepresentable<KeyType>::Value >
		struct KeyNotFoundExceptionCreator
		{ static KeyNotFoundException Create(const KeyType& key) { return KeyNotFoundException(ToString(key)); } };

		template < typename KeyType >
		struct KeyNotFoundExceptionCreator<KeyType, false>
		{ static KeyNotFoundException Create(const KeyType& key) { return KeyNotFoundException(); } };
	}

	template < typename KeyType >
	KeyNotFoundException CreateKeyNotFoundException(const KeyType& key)
	{ return Detail::KeyNotFoundExceptionCreator<KeyType>::Create(key); }

	/** @} */

}


#endif
