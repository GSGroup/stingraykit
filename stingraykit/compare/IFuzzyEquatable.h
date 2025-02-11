#ifndef STINGRAYKIT_COMPARE_IFUZZYEQUATABLE_H
#define STINGRAYKIT_COMPARE_IFUZZYEQUATABLE_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/compare/comparers.h>

namespace stingray
{

	struct IFuzzyEquatable
	{
		virtual ~IFuzzyEquatable() { }

		virtual bool FuzzyEquals(const IFuzzyEquatable& other) const = 0;
	};


	template < typename T >
	struct FuzzyEquatable : public virtual IFuzzyEquatable
	{
		bool FuzzyEquals(const IFuzzyEquatable& other) const override
		{
			const std::type_info& thisType = typeid(*this);
			const std::type_info& otherType = typeid(other);

			if (thisType != otherType)
				return false;

			//avoiding dynamic_cast here
			const char* thisPtr = reinterpret_cast<const char*>(this);
			const char* thisIFuzzyEquatable = reinterpret_cast<const char*>(static_cast<const IFuzzyEquatable*>(this));
			const ptrdiff_t delta = thisPtr - thisIFuzzyEquatable; //distance between FuzzyEquatable and IFuzzyEquatable for this type.
			const char* otherIFuzzyEquatable = reinterpret_cast<const char*>(&other);
			const FuzzyEquatable<T>* otherPtr = reinterpret_cast<const FuzzyEquatable<T>*>(otherIFuzzyEquatable + delta);

			return DoFuzzyEquals(*static_cast<const T*>(otherPtr));
		}

	protected:
		virtual bool DoFuzzyEquals(const T& other) const = 0;
	};


	struct FuzzyEquals : public comparers::EqualsComparerBase<FuzzyEquals>
	{
		template < typename T >
		auto DoCompare(const T& lhs, const T& rhs) const
				-> decltype(lhs.FuzzyEquals(rhs), bool())
		{
			if (&lhs == &rhs)
				return true;

			return lhs.FuzzyEquals(rhs);
		}
	};

}

#endif
