#ifndef STINGRAYKIT_COMPARE_COMPARABLE_H
#define STINGRAYKIT_COMPARE_COMPARABLE_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <typeinfo>

#include <stddef.h>

namespace stingray
{

	struct IComparable
	{
		virtual ~IComparable() { }

		virtual int Compare(const IComparable& other) const = 0;
	};


	template < typename T >
	struct Comparable : public virtual IComparable
	{
		virtual int Compare(const IComparable& other) const
		{
			if (this == &other)
				return 0;

			const std::type_info& thisType = typeid(*this);
			const std::type_info& otherType = typeid(other);

			if (thisType != otherType)
				return thisType.before(otherType) ? -1 : 1;

			//avoiding dynamic_cast here
			const char* thisPtr = reinterpret_cast<const char*>(this);
			const char* thisIComparable = reinterpret_cast<const char*>(static_cast<const IComparable*>(this));
			const ptrdiff_t delta = thisPtr - thisIComparable; //distance between Comparable and IComparable for this type.
			const char* otherIComparable = reinterpret_cast<const char*>(&other);
			const Comparable<T>* otherPtr = reinterpret_cast<const Comparable<T>*>(otherIComparable + delta);

			return DoCompare(*static_cast<const T*>(otherPtr));
		}

	protected:
		virtual int DoCompare(const T& other) const = 0;
	};

}

#endif
