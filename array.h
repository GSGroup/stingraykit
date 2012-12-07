#ifndef STINGRAY_TOOLKIT_ARRAY_H
#define STINGRAY_TOOLKIT_ARRAY_H

#include <stddef.h>
#include <algorithm>
#include <stdexcept>

namespace stingray
{
	template<typename T, size_t N>
	class array
	{
        T					_data[N];

	public:
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

		array() {}
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

	private:
		static void _check_index(size_type off)
		{
			if (off >= N)
				throw std::range_error("array index is out of range");
		}
	};
}

#endif
