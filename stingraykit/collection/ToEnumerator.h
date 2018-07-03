#ifndef STINGRAYKIT_COLLECTION_TOENUMERATOR_H
#define STINGRAYKIT_COLLECTION_TOENUMERATOR_H

// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/shared_ptr.h>

namespace stingray
{

	namespace Detail
	{
		template< typename T, typename Enabler = void >
		struct ToEnumeratorImpl;
	}

	template<typename T>
	shared_ptr<typename Detail::ToEnumeratorImpl<T>::ValueT> ToEnumerator(const shared_ptr<T>& src)
	{ return Detail::ToEnumeratorImpl<T>::Do(src); }

}

#endif
