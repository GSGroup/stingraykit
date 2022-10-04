#ifndef STINGRAYKIT_COLLECTION_ITERATOR_BASE_H
#define STINGRAYKIT_COLLECTION_ITERATOR_BASE_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <iterator>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	/**
	 * @brief a helper base class for iterators
	 * @par methods to implement:
	 * @code
	 * typename base::reference dereference() const;
	 * bool equal(const const_iterator& other) const;
	 *
	 * // std::forward_iterator_tag
	 * void increment();
	 *
	 * // std::bidirectional_iterator_tag
	 * void decrement();
	 *
	 * //std::random_access_iterator_tag
	 * void advance(const typename base::difference_type& diff);
	 * typename base::difference_type distance_to(const const_iterator& other) const;
	 * @endcode
	 */

	template < typename Derived_, typename ValueType_, typename Category_,
			typename Distance_ = std::ptrdiff_t, typename Pointer_ = ValueType_*, typename Reference_ = ValueType_& >
	class iterator_base : public std::iterator<Category_, ValueType_, Distance_, Pointer_, Reference_>
	{
		using base = std::iterator<Category_, ValueType_, Distance_, Pointer_, Reference_>;

		using Derived = Derived_;

	public:
		using iterator_category = typename base::iterator_category;
		using value_type = typename base::value_type;
		using difference_type = typename base::difference_type;
		using pointer = typename base::pointer;
		using reference = typename base::reference;

	public:
		// Forward iterator requirements
		const reference operator * () const
		{ return GetDerived().dereference(); }

		reference operator * ()
		{ return GetDerived().dereference(); }

		pointer operator -> ()
		{ return &GetDerived().dereference(); }

		const pointer operator -> () const
		{ return &GetDerived().dereference(); }

		Derived& operator ++ ()
		{
			GetDerived().increment();
			return GetDerived();
		}

		Derived operator ++ (int)
		{
			Derived result(GetDerived());
			GetDerived().increment();
			return result;
		}

		// Bidirectional iterator requirements
		Derived& operator -- ()
		{
			GetDerived().decrement();
			return GetDerived();
		}

		Derived operator -- (int)
		{
			Derived result(GetDerived());
			GetDerived().decrement();
			return result;
		}

		// Random access iterator requirements
		reference operator [] (const difference_type& diff) const
		{
			Derived result(GetDerived());
			result.advance(diff);
			return result.dereference();
		}

		Derived& operator += (const difference_type& diff)
		{
			GetDerived().advance(diff);
			return GetDerived();
		}

		Derived operator + (const difference_type& diff) const
		{
			Derived result(GetDerived());
			return result += diff;
		}

		Derived& operator -= (const difference_type& diff)
		{ return GetDerived() += -diff; }

		Derived operator-(const difference_type& diff) const
		{
			Derived result(GetDerived());
			return result -= diff;
		}

		difference_type operator - (const Derived& other) const
		{ return other.distance_to(GetDerived()); }

		// Forward iterator requirements
		bool operator == (const Derived& other) const
		{ return GetDerived().equal(other); }

		bool operator != (const Derived& other) const
		{ return !(*this == other); }

		// Random access iterator requirements
		bool operator < (const Derived& other) const
		{ return GetDerived().distance_to(other) > 0; }

		bool operator > (const Derived& other) const
		{ return other < GetDerived(); }

		bool operator <= (const Derived& other) const
		{ return !(other < GetDerived()); }

		bool operator >= (const Derived& other) const
		{ return !(*this < other); }

	protected:
		~iterator_base() { }

	private:
		Derived& GetDerived()
		{ return static_cast<Derived&>(*this); }

		const Derived& GetDerived() const
		{ return static_cast<const Derived&>(*this); }
	};

	/** @} */
}

#endif
