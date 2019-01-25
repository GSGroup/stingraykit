#ifndef STINGRAYKIT_COLLECTION_ARRAY_H
#define STINGRAYKIT_COLLECTION_ARRAY_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/toolkit.h>

#include <algorithm>
#include <stdexcept>

#include <stddef.h>

namespace stingray
{
	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template<typename T, size_t N>
	class array
	{
		T					_data[N];

	public:
		static const size_t Size = N;
		typedef T			value_type;
		typedef T*			iterator;
		typedef const T*	const_iterator;
		typedef T&			reference;
		typedef const T&	const_reference;
		typedef size_t		size_type;
		typedef ptrdiff_t	difference_type;

		const_iterator begin() const	{ return _data; }
		const_iterator cbegin() const	{ return _data; }
		iterator begin()				{ return _data; }

		const_iterator end() const	{ return _data + N; }
		const_iterator cend() const	{ return _data + N; }
		iterator end()				{ return _data + N; }

		array(): _data() {}
		void assign(const array &other)			{ std::copy(other.begin(), other.end(), begin()); }
		array(const array& other)				{ assign(other); }
		array& operator=(const array& other)	{ assign(other); return *this; }

		reference front()						{ return _data[0]; }
		const_reference front() const			{ return _data[0]; }

		reference back()						{ return _data[N - 1]; }
		const_reference back() const			{ return _data[N - 1]; }

		value_type *data()						{ return _data; }
		const value_type *data() const			{ return _data; }

		reference at(size_type off)				{ _check_index(off); return _data[off]; }
		const_reference at(size_type off) const	{ _check_index(off); return _data[off]; }
		reference operator[](size_type off)		{ return _data[off]; }
		const_reference operator[](size_type off) const { return _data[off]; }

		void fill(const value_type & value)		{ std::fill(begin(), end(), value); }

		static size_type size()					{ return N; }
		static size_type max_size()				{ return N; }
		static bool empty()						{ return N == 0; }

		void swap(array &other)					{ for(size_type i = 0; i < N; ++i) std::swap(_data[i], other._data[i]); }

		bool operator == (const array& other) const { return std::equal(data(), data() + size(), other.data()); }
		bool operator != (const array& other) const { return !(*this == other); }

		bool operator < (const array& other) const
		{ return std::lexicographical_compare(data(), data() + size(), other.data(), other.data() + other.size()); }

		STINGRAYKIT_GENERATE_RELATIONAL_OPERATORS_FROM_LESS(array);

	private:
		static void _check_index(size_type off)
		{
			if (off >= N)
				throw std::range_error("array index is out of range");
		}
	};

	/** @} */
}

#endif
