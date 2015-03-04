#ifndef STINGRAYKIT_COLLECTION_ITERATOR_BASE_H
#define STINGRAYKIT_COLLECTION_ITERATOR_BASE_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
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
	 * typename base::difference_type distance_to(const const_iterator &other) const;
	 * @endcode
	 */
	template<typename Derived_t, typename T, typename Category_t,
		typename Distance_t = std::ptrdiff_t, typename Pointer_t = T*, typename Reference_t = T&>
	class iterator_base : public std::iterator<Category_t, T, Distance_t, Pointer_t, Reference_t>
	{
		typedef std::iterator<Category_t, T, Distance_t, Pointer_t, Reference_t>	base;

	protected:
		~iterator_base() { }

	private:
		Derived_t& GetDerived()
		{ return static_cast<Derived_t&>(*this); }

		const Derived_t& GetDerived() const
		{ return static_cast<const Derived_t&>(*this); }

	public:
		typedef typename base::iterator_category	iterator_category;
		typedef typename base::value_type			value_type;
		typedef typename base::difference_type		difference_type;
		typedef typename base::pointer				pointer;
		typedef	typename base::reference			reference;

		// Forward iterator requirements
		const reference operator*() const
		{ return GetDerived().dereference(); }

		reference operator*()
		{ return GetDerived().dereference(); }

		pointer operator->()
		{ return &(GetDerived().dereference()); }

		const pointer operator->() const
		{ return &(GetDerived().dereference()); }


		Derived_t& operator++()
		{
			GetDerived().increment();
			return GetDerived();
		}

		Derived_t operator++(int)
		{
			Derived_t result(GetDerived());
			GetDerived().increment();
			return result;
		}

		// Bidirectional iterator requirements
		Derived_t& operator--()
		{
			GetDerived().decrement();
			return GetDerived();
		}

		Derived_t operator--(int)
		{
			Derived_t result(GetDerived());
			GetDerived().decrement();
			return result;
		}

		// Random access iterator requirements
		reference operator[](const difference_type& diff) const
		{
			Derived_t result(GetDerived());
			result.advance(diff);
			return result.dereference();
		}

		Derived_t& operator+=(const difference_type& diff)
		{
			GetDerived().advance(diff);
			return GetDerived();
		}

		Derived_t operator+(const difference_type& diff) const
		{
			Derived_t result(GetDerived());
			return result += diff;
		}

		Derived_t& operator-=(const difference_type& diff)
		{ return GetDerived() += (-diff); }

		Derived_t operator-(const difference_type& diff) const
		{
			Derived_t result(GetDerived());
			return result -= diff;
		}

		difference_type operator-(const Derived_t &other) const
		{ return other.distance_to(GetDerived()); }

		bool operator==(const Derived_t &other) const
		{ return GetDerived().equal(other); }

		bool operator!=(const Derived_t &other) const
		{ return !(*this == other); }

		bool operator<(const Derived_t &other) const
		{ return GetDerived().distance_to(other) > 0; }

		bool operator<=(const Derived_t &other) const
		{ return ((*this < other) || (*this == other)); }

		bool operator>(const Derived_t &other) const
		{ return !((*this < other) || (*this == other)); }

		bool operator>=(const Derived_t &other) const
		{ return !(*this < other); }
	};

	/** @} */
}

#endif
