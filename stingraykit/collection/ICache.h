#ifndef STINGRAYKIT_COLLECTION_ICACHE_H
#define STINGRAYKIT_COLLECTION_ICACHE_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/signal/signal_connector.h>

namespace stingray
{

	template < typename Key_, typename Value_ >
	struct ICache
	{
		using KeyPassingType = typename GetParamPassingType<Key_>::ValueT;
		using ValuePassingType = typename GetParamPassingType<Value_>::ValueT;

		using OnEvictedSignature = void (KeyPassingType, ValuePassingType);

	public:
		virtual ~ICache() { }

		virtual bool TryGet(KeyPassingType key, Value_& out) = 0;
		virtual void Set(KeyPassingType key, ValuePassingType value) = 0;
		virtual bool Remove(KeyPassingType key) = 0;

		virtual void Clear() = 0;

		virtual size_t GetSize() const = 0;

		virtual signal_connector<OnEvictedSignature> OnEvicted() const = 0;
	};

}

#endif
