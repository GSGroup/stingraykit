#ifndef STINGRAYKIT_COLLECTION_ISET_H
#define STINGRAYKIT_COLLECTION_ISET_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/ICollection.h>

#define STINGRAYKIT_DECLARE_SET(ClassName) \
		using ClassName##Set = stingray::ISet<ClassName>; \
		STINGRAYKIT_DECLARE_PTR(ClassName##Set); \
		using ClassName##ReadonlySet = stingray::IReadonlySet<ClassName>; \
		STINGRAYKIT_DECLARE_PTR(ClassName##ReadonlySet); \
		STINGRAYKIT_DECLARE_COLLECTION(ClassName)

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename T >
	struct IReadonlySet : public virtual ICollection<T>, public virtual IReversableEnumerable<T>
	{
		using ValueType = T;

		~IReadonlySet() override { }

		virtual bool Contains(const ValueType& value) const = 0;

		virtual shared_ptr<IEnumerator<T>> Find(const T& value) const = 0;
		virtual shared_ptr<IEnumerator<T>> ReverseFind(const T& value) const = 0;
	};


	template < typename T >
	struct IsInheritedIReadonlySet : public IsInheritedFromTemplate<T, IReadonlySet>
	{ };


	template < typename T >
	struct ISet : public virtual IReadonlySet<T>
	{
		using ValueType = T;

		~ISet() override { }

		virtual bool Add(const ValueType& value) = 0;

		virtual bool Remove(const ValueType& value) = 0;
		virtual size_t RemoveWhere(const function<bool (const ValueType&)>& pred) = 0;

		virtual void Clear() = 0;
	};


	template < typename T >
	struct IsInheritedISet : public IsInheritedFromTemplate<T, ISet>
	{ };

	/** @} */

}

#endif
