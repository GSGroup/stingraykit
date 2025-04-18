#ifndef STINGRAYKIT_COLLECTION_RANGEBASE_H
#define STINGRAYKIT_COLLECTION_RANGEBASE_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/toolkit.h>

#include <iterator>

namespace stingray
{

	struct RangeMarker
	{
	protected:
		RangeMarker()
		{ }
	};


	template < typename T >
	struct IsRange : IsInherited<T, RangeMarker> { };


	namespace Range
	{
		template < typename T >
		struct ArrowProxy
		{
		private:
			T _val;

		public:
			ArrowProxy(T val) : _val(val)
			{ }

			typename ToPointer<T>::ValueT operator -> ()
			{ return to_pointer(_val); }
		};


		template < typename Derived_, typename ValueType_, typename Category_ >
		struct RangeBase :
			public std::iterator<Category_, ValueType_, std::ptrdiff_t, ArrowProxy<ValueType_>, ValueType_>,
			public RangeMarker
		{
		private:
			typedef Derived_   Derived;

		public:
			typedef Category_  Category;
			typedef ValueType_ ValueType;

			static const bool ReturnsTemporary = false;

		public:
			ValueType		operator *  () const						{ return GetDerived().Get(); }
			ArrowProxy<ValueType> operator -> () const					{ return GetDerived().Get(); }

			Derived&		operator ++ ()								{ GetDerived().Next(); return GetDerived(); }
			Derived			operator ++ (int)							{ Derived result(GetDerived()); GetDerived().Next(); return result; }

			bool			operator == (const Derived& other) const	{ return GetDerived().Equals(other); }
			bool			operator != (const Derived& other) const	{ return !GetDerived().Equals(other); }

			Derived&		operator -- ()								{ GetDerived().Prev(); return GetDerived(); }
			Derived			operator -- (int)							{ Derived result(GetDerived()); GetDerived().Prev(); return result; }

			ValueType		operator [] (std::ptrdiff_t index) const	{ static_assert(sizeof(ValueType_) < 0, "No brackets operator"); }
			Derived&		operator += (std::ptrdiff_t distance)		{ GetDerived().Move(distance); return GetDerived(); }
			Derived&		operator -= (std::ptrdiff_t distance)		{ GetDerived().Move(-distance); return GetDerived(); }
			Derived			operator + (std::ptrdiff_t distance) const	{ Derived result(GetDerived()); return result += distance; }
			Derived			operator - (std::ptrdiff_t distance) const	{ Derived result(GetDerived()); return result -= distance; }
			std::ptrdiff_t	operator - (const Derived& other) const		{ return GetDerived().GetPosition() - other.GetPosition(); }

			Derived begin() const										{ Derived r(GetDerived()); return r.First(); }
			Derived end() const											{ Derived r(GetDerived()); return r.End(); }

			explicit operator bool () const								{ return GetDerived().Valid(); }

		private:
			Derived& GetDerived()										{ return *static_cast<Derived*>(this); }
			const Derived& GetDerived() const							{ return *static_cast<const Derived*>(this); }
		};
	}
}

#endif
