#ifndef STINGRAYKIT_COLLECTION_TORANGE_H
#define STINGRAYKIT_COLLECTION_TORANGE_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

namespace stingray
{

	namespace Detail
	{
		template <typename T, typename Enabler = void>
		struct ToRangeImpl;
	}


	template <typename T>
	typename Detail::ToRangeImpl<T>::ValueT ToRange(T& src)
	{ return Detail::ToRangeImpl<T>::Do(src); }


	template <typename T>
	typename Detail::ToRangeImpl<const T>::ValueT ToRange(const T& src)
	{ return Detail::ToRangeImpl<const T>::Do(src); }
}

#endif
