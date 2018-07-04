#ifndef STINGRAYKIT_COLLECTION_ILIST_H
#define STINGRAYKIT_COLLECTION_ILIST_H

// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/ICollection.h>
#include <stingraykit/optional.h>

#define STINGRAYKIT_DECLARE_LIST(ClassName) \
		typedef stingray::IList<ClassName>				ClassName##List; \
		STINGRAYKIT_DECLARE_PTR(ClassName##List); \
		STINGRAYKIT_DECLARE_COLLECTION(ClassName)

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename T >
	struct IReadonlyList : public virtual ICollection<T>, public virtual IReversableEnumerable<T>
	{
		typedef T	ValueType;

		virtual ~IReadonlyList() { }

		virtual bool Contains(const ValueType& value) const = 0;
		virtual optional<size_t> IndexOf(const ValueType& value) const = 0;

		virtual ValueType Get(size_t index) const = 0;
		virtual bool TryGet(size_t index, ValueType& value) const = 0;
	};


	template < typename T >
	struct IsInheritedIReadonlyList : public IsInherited1ParamTemplate<T, IReadonlyList>
	{ };


	template < typename T >
	struct IList : public virtual IReadonlyList<T>
	{
		typedef T	ValueType;

		virtual ~IList() { }

		virtual void Add(const ValueType& value) = 0;
		virtual void Set(size_t index, const ValueType& value) = 0;
		virtual void Insert(size_t index, const ValueType& value) = 0;

		virtual void RemoveAt(size_t index) = 0;
		virtual void Remove(const ValueType& value) = 0;
		virtual size_t RemoveAll(const function<bool (const ValueType&)>& pred) = 0;

		virtual void Clear() = 0;
	};


	template < typename T >
	struct IsInheritedIList : public IsInherited1ParamTemplate<T, IList>
	{ };

	/** @} */

}

#endif
