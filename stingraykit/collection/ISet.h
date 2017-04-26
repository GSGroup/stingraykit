#ifndef STINGRAYKIT_COLLECTION_ISET_H
#define STINGRAYKIT_COLLECTION_ISET_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/collection/ICollection.h>


#define STINGRAYKIT_DECLARE_SET(ClassName) \
		typedef stingray::ISet<ClassName> ClassName##Set; \
		STINGRAYKIT_DECLARE_PTR(ClassName##Set); \
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
		typedef T	ValueType;

		virtual ~IReadonlySet() { }

		virtual bool Contains(const ValueType& value) const = 0;
	};


	template < typename T >
	struct InheritsIReadonlySet : public Inherits1ParamTemplate<T, IReadonlySet>
	{ };


	template < typename T >
	struct ISet : public virtual IReadonlySet<T>
	{
		typedef T	ValueType;

		virtual ~ISet() { }

		virtual void Add(const ValueType& value) = 0;
		virtual void Clear() = 0;
		virtual void Remove(const ValueType& value) = 0;
		virtual bool TryRemove(const ValueType& value) = 0;
	};


	template < typename T >
	struct InheritsISet : public Inherits1ParamTemplate<T, ISet>
	{ };

	/** @} */

}


#endif
