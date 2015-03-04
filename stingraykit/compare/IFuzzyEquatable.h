#ifndef STINGRAYKIT_COMPARE_IFUZZYEQUATABLE_H
#define STINGRAYKIT_COMPARE_IFUZZYEQUATABLE_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <typeinfo>

#include <stingraykit/shared_ptr.h>


namespace stingray
{

	struct IFuzzyEquatable
	{
		virtual ~IFuzzyEquatable() {}
		virtual bool FuzzyEquals(const IFuzzyEquatable &other) const = 0;
	};


	template<typename T>
	struct FuzzyEquatable : public virtual IFuzzyEquatable
	{
		virtual bool FuzzyEquals(const IFuzzyEquatable &other) const
		{
			const std::type_info &my_type = typeid(*this);
			const std::type_info &other_type = typeid(other);

			if (my_type != other_type)
				return false;

			//avoiding dynamic_cast here
			const char * this_ptr = reinterpret_cast<const char *>(this);
			const char * this_ifuzzyequatable = reinterpret_cast<const char *>(static_cast<const IFuzzyEquatable *>(this));
			ptrdiff_t delta = this_ptr - this_ifuzzyequatable; //distance between FuzzyEquatable and IFuzzyEquatable for this type.
			const char * other_ifuzzyequatable = reinterpret_cast<const char *>(&other);
			const FuzzyEquatable<T> *other_ptr = reinterpret_cast<const FuzzyEquatable<T> *>(other_ifuzzyequatable + delta);

			return this->DoFuzzyEquals(*static_cast<const T*>(other_ptr));
		}

	protected:
		virtual bool DoFuzzyEquals(const T &other) const = 0;
	};


	struct FuzzyEqualityComparer : std::binary_function<const shared_ptr<const IFuzzyEquatable> &, const shared_ptr<const IFuzzyEquatable> &, bool>
	{
		typedef const shared_ptr<const IFuzzyEquatable> & PointerType;

		inline bool operator()(PointerType l, PointerType r) const								{ return FuzzyEquals(l, r); }
		inline bool operator()(const IFuzzyEquatable& l, const IFuzzyEquatable& r) const		{ return FuzzyEquals(l, r); }

		static inline bool FuzzyEquals(PointerType l, PointerType r)							{ return (!l && !r) || FuzzyEquals(*l, *r); }
		static inline bool FuzzyEquals(const IFuzzyEquatable& l, const IFuzzyEquatable& r)		{ return l.FuzzyEquals(r); }
	};

}

#endif
