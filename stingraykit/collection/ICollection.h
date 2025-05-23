#ifndef STINGRAYKIT_COLLECTION_ICOLLECTION_H
#define STINGRAYKIT_COLLECTION_ICOLLECTION_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/IEnumerable.h>

#define STINGRAYKIT_DECLARE_COLLECTION(ClassName) \
		using ClassName##Collection = stingray::ICollection<ClassName>; \
		STINGRAYKIT_DECLARE_PTR(ClassName##Collection); \
		STINGRAYKIT_DECLARE_ENUMERABLE(ClassName)

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename T >
	struct ICollection : public virtual IEnumerable<T>
	{
		~ICollection() override { }

		virtual size_t GetCount() const = 0;
		virtual bool IsEmpty() const = 0;
	};

	/** @} */

}

#endif
