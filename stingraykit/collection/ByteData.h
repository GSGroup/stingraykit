#ifndef STINGRAYKIT_COLLECTION_BYTEDATA_H
#define STINGRAYKIT_COLLECTION_BYTEDATA_H

// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/array.h>
#include <stingraykit/collection/iterator_base.h>
#include <stingraykit/shared_ptr.h>

#include <algorithm>

#define DETAIL_BYTEDATA_INDEX_CHECK(Arg1, Arg2) STINGRAYKIT_CHECK((Arg1) <= (Arg2), IndexOutOfRangeException(Arg1, Arg2))


namespace stingray
{
	/**
	 * @ingroup toolkit_bits
	 * @defgroup toolkit_bits_bytedata ByteData and ByteArray
	 * @{
	 */

	template < typename T >
	class ByteDataIterator : public iterator_base<ByteDataIterator<T>, T, std::random_access_iterator_tag>
	{
		typedef iterator_base<ByteDataIterator, T, std::random_access_iterator_tag>		base;

		template < typename U >
		friend class ByteDataIterator;

	public:
		typedef typename base::pointer													pointer;
		typedef typename base::reference												reference;
		typedef typename base::difference_type											difference_type;

	private:
		pointer _ptr;
		pointer _begin;
		pointer _end;

	public:
		ByteDataIterator() : _ptr(), _begin(), _end() { }

		ByteDataIterator(const pointer ptr, const pointer begin, const pointer end)
			:	_ptr(ptr),
				_begin(begin),
				_end(end)
		{ STINGRAYKIT_CHECK(_ptr >= _begin && _ptr <= _end, IndexOutOfRangeException((u64)_ptr, (u64)_begin, (u64)_end)); }

		template < typename U >
		ByteDataIterator(const ByteDataIterator<U>& other)
			:	_ptr(other._ptr),
				_begin(other._begin),
				_end(other._end)
		{ STINGRAYKIT_CHECK(_ptr >= _begin && _ptr <= _end, IndexOutOfRangeException((u64)_ptr, (u64)_begin, (u64)_end)); }

		reference dereference() const
		{
			STINGRAYKIT_CHECK(_ptr >= _begin && _ptr < _end, IndexOutOfRangeException((u64)_ptr, (u64)_begin, (u64)_end));
			return *_ptr;
		}

		bool equal(const ByteDataIterator& other) const						{ return _ptr == other._ptr; }
		void increment()													{ ++_ptr; }
		void decrement()													{ --_ptr; }
		void advance(difference_type n)										{ _ptr += n; }
		difference_type distance_to(const ByteDataIterator& other) const	{ return other._ptr - _ptr; }
	};


	namespace Detail
	{
#if !defined(PRODUCTION_BUILD)
		template<typename T>
		struct ByteDataIteratorSelector
		{
			typedef ByteDataIterator<T>			iterator;
			typedef ByteDataIterator<const T>	const_iterator;

			static iterator CreateIterator(T* ptr, T* begin, T* end)
			{ return iterator(ptr, begin, end); }
			static const_iterator CreateConstIterator(const T* ptr, const T* begin, const T* end)
			{ return const_iterator(ptr, begin, end); }
		};
#else
		template<typename T>
		struct ByteDataIteratorSelector
		{
			typedef T*			iterator;
			typedef const T*	const_iterator;

			static iterator CreateIterator(T* ptr, T* begin, T* end)
			{ (void)begin; (void)end; return iterator(ptr); }
			static const_iterator CreateConstIterator(const T* ptr, const T* begin, const T* end)
			{ (void)begin; (void)end; return const_iterator(ptr); }
		};
#endif
	}

	template < typename T >
	class BasicByteData;

	/**
	 * @brief An object that retains shared ownership of an array of bytes
	 */
	template < typename T >
	class BasicByteArray
	{
		STINGRAYKIT_DEFAULTCOPYABLE(BasicByteArray);
		STINGRAYKIT_DEFAULTMOVABLE(BasicByteArray);

		template < typename U >
		friend class BasicByteArray;

		typedef typename RemoveConst<T>::ValueT NonConstType;

	public:
		typedef typename
			If<
				IsConst<T>::Value,
				const std::vector<NonConstType>,
				std::vector<T>
			>::ValueT	CollectionType;

		typedef shared_ptr<CollectionType>		CollectionTypePtr;

	private:
		static const size_t NoSizeLimit = ~(size_t)0;

	private:
		CollectionTypePtr		_data;
		size_t					_offset;
		size_t					_sizeLimit;

	public:
		typedef T																value_type;

		typedef typename Detail::ByteDataIteratorSelector<T>::iterator			iterator;
		typedef typename Detail::ByteDataIteratorSelector<T>::const_iterator	const_iterator;

		typedef std::reverse_iterator<iterator>									reverse_iterator;
		typedef std::reverse_iterator<const_iterator>							const_reverse_iterator;

		BasicByteArray()
			: _data(make_shared_ptr<CollectionType>()), _offset(0), _sizeLimit(NoSizeLimit)
		{ }

		BasicByteArray(const CollectionTypePtr& data)
			: _data(data), _offset(0), _sizeLimit(NoSizeLimit)
		{ }

		explicit BasicByteArray(size_t size)
			: _data(make_shared_ptr<CollectionType>(size)), _offset(0), _sizeLimit(NoSizeLimit)
		{ }

		BasicByteArray(const T* data, size_t size)
			: _data(make_shared_ptr<CollectionType>(data, data + size)), _offset(0), _sizeLimit(NoSizeLimit)
		{ }

		template < typename Range, decltype(std::declval<Range>().begin(), std::declval<Range>().end(), bool()) = false >
		explicit BasicByteArray(const Range& range)
			: _data(make_shared_ptr<CollectionType>(range.begin(), range.end())), _offset(0), _sizeLimit(NoSizeLimit)
		{ }

		template < typename InputIterator >
		BasicByteArray(InputIterator first, InputIterator last)
			: _data(make_shared_ptr<CollectionType>(first, last)), _offset(0), _sizeLimit(NoSizeLimit)
		{ }

		template < typename U, typename EnableIf<IsConvertible<U*, T*>::Value, bool>::ValueT = false >
		BasicByteArray(const BasicByteArray<U>& other)
			: _data(other._data), _offset(other._offset), _sizeLimit(other._sizeLimit)
		{ }

		template < typename U, typename EnableIf<IsConvertible<U*, T*>::Value, bool>::ValueT = false >
		BasicByteArray(const BasicByteArray<U>& other, size_t offset)
			: _data(other._data), _offset(other._offset + offset), _sizeLimit(other._sizeLimit == NoSizeLimit ? NoSizeLimit : other._sizeLimit - offset)
		{ STINGRAYKIT_CHECK(_data->size() >= _offset, IndexOutOfRangeException(_offset, _data->size())); }

		template < typename U, typename EnableIf<IsConvertible<U*, T*>::Value, bool>::ValueT = false >
		BasicByteArray(const BasicByteArray<U>& other, size_t offset, size_t sizeLimit)
			: _data(other._data), _offset(other._offset + offset), _sizeLimit(sizeLimit)
		{
			STINGRAYKIT_CHECK(_data->size() >= _offset, IndexOutOfRangeException(_offset, _data->size()));
			STINGRAYKIT_CHECK(_sizeLimit == NoSizeLimit || _sizeLimit + offset <= _data->size(), IndexOutOfRangeException(_sizeLimit + offset, offset, _data->size()));
			STINGRAYKIT_CHECK(_sizeLimit + offset <= other._sizeLimit, IndexOutOfRangeException(_sizeLimit + offset, offset, other._sizeLimit));
		}

		void RequireSize(size_t size)
		{
			STINGRAYKIT_CHECK(_sizeLimit == NoSizeLimit, NotImplementedException());
			if (_data->size() < size + _offset)
				_data->resize(size + _offset);
		}

		T& operator [] (size_t index) const
		{
			STINGRAYKIT_CHECK(index < size(), IndexOutOfRangeException(index, size()));
			return (*_data)[index + _offset];
		}

		size_t size() const
		{ return _data->size() >= _offset ? std::min(_data->size() - _offset, _sizeLimit) : 0; }

		bool empty() const
		{ return size() == 0; }

		template < typename InputIterator >
		void append(InputIterator first, InputIterator last)
		{
			STINGRAYKIT_CHECK(_sizeLimit == NoSizeLimit, NotImplementedException());
			_data->insert(_data->end(), first, last);
		}

		template < typename Range, decltype(std::declval<Range>().begin(), std::declval<Range>().end(), bool()) = false >
		void append(const Range& range)
		{ append(range.begin(), range.end()); }

		template < typename U >
		void append(const BasicByteArray<U>& other)
		{ append(other.data(), other.data() + other.size()); }

		void reserve(size_t n)
		{
			STINGRAYKIT_CHECK(_sizeLimit == NoSizeLimit, NotImplementedException());
			_data->reserve(_offset + n);
		}

		void swap(BasicByteArray& other)
		{
			_data.swap(other._data);
			std::swap(_offset, other._offset);
			std::swap(_sizeLimit, other._sizeLimit);
		}

		void clear()
		{ _data->clear(); }

		iterator begin()
		{
			T* data = this->data();
			return Detail::ByteDataIteratorSelector<T>::CreateIterator(data, data, data + size());
		}

		iterator end()
		{
			T* data = this->data();
			return Detail::ByteDataIteratorSelector<T>::CreateIterator(data + size(), data, data + size());
		}

		const_iterator begin() const
		{
			const T* data = this->data();
			return Detail::ByteDataIteratorSelector<T>::CreateConstIterator(data, data, data + size());
		}

		const_iterator end() const
		{
			const T* data = this->data();
			return Detail::ByteDataIteratorSelector<T>::CreateConstIterator(data + size(), data, data + size());
		}

		reverse_iterator rbegin()
		{ return reverse_iterator(end()); }

		reverse_iterator rend()
		{ return reverse_iterator(begin()); }

		const_reverse_iterator rbegin() const
		{ return const_reverse_iterator(end()); }

		const_reverse_iterator rend() const
		{ return const_reverse_iterator(begin()); }

		T* data() const
		{ return empty() ? NULL : &(*_data)[_offset]; }

		template < typename ObjectOStream >
		void Serialize(ObjectOStream& ar) const
		{ ar.Serialize("d", BasicByteData<T>(*this)); }

		template < typename ObjectIStream >
		void Deserialize(ObjectIStream& ar)
		{
			BasicByteArray<NonConstType> data;
			ar.Deserialize("o", data._offset, 0);
			ar.Deserialize("d", *data._data);
			*this = data;
		}

		std::string ToString() const
		{
			string_ostream stream;
			stream << "BasicByteArray<" << TypeInfo(typeid(T)).GetName() << "> { size: " << stingray::ToString(size()) << " }";
			return stream.str();
		}

		template < typename U >
		bool operator == (const BasicByteArray<U>& other) const
		{ return size() == other.size() && std::equal(data(), data() + size(), other.data()); }

		STINGRAYKIT_GENERATE_EQUALITY_OPERATORS_FROM_EQUAL(BasicByteArray);

		bool operator < (const BasicByteArray& other) const
		{ return std::lexicographical_compare(data(), data() + size(), other.data(), other.data() + other.size()); }

		STINGRAYKIT_GENERATE_RELATIONAL_OPERATORS_FROM_LESS(BasicByteArray);
	};


	/**
	 * @brief An object that holds a reference to an array of bytes but does not retain any ownership
	 */
	template < typename T >
	class BasicByteData
	{
	private:
		T*		_data;
		size_t	_size;

	public:
		typedef T																value_type;

		typedef typename Detail::ByteDataIteratorSelector<T>::iterator			iterator;
		typedef typename Detail::ByteDataIteratorSelector<T>::const_iterator	const_iterator;

		typedef std::reverse_iterator<iterator>									reverse_iterator;
		typedef std::reverse_iterator<const_iterator>							const_reverse_iterator;

		BasicByteData()
			: _data(), _size()
		{ }

		template < typename U, typename EnableIf<IsConvertible<U*, T*>::Value, bool>::ValueT = false >
		BasicByteData(BasicByteData<U> data)
			: _data(data.empty() ? NULL : &data[0]), _size(data.size())
		{ }

		template < typename U >
		BasicByteData(std::vector<U>& data)
			: _data(data.empty() ? NULL : &data[0]), _size(data.size())
		{ }

		template < typename U, typename EnableIf<IsConvertible<const U*, T*>::Value, bool>::ValueT = false >
		BasicByteData(const std::vector<U>& data)
			: _data(data.empty() ? NULL : &data[0]), _size(data.size())
		{ }

		template < typename U, size_t N >
		BasicByteData(array<U, N>& data)
			: _data(data.empty() ? NULL : &data[0]), _size(data.size())
		{ }

		template < typename U, size_t N, typename EnableIf<IsConvertible<const U*, T*>::Value, bool>::ValueT = false >
		BasicByteData(const array<U, N>& data)
			: _data(data.empty() ? NULL : &data[0]), _size(data.size())
		{ }

		template < typename U, typename EnableIf<IsConvertible<U*, T*>::Value, bool>::ValueT = false >
		BasicByteData(const BasicByteArray<U>& array)
			: _data(array.data()), _size(array.size())
		{ }

		template < typename U, typename EnableIf<IsConvertible<U*, T*>::Value, bool>::ValueT = false >
		BasicByteData(const BasicByteArray<U>& array, size_t offset)
			: _data(array.data() + offset), _size(array.size() - offset)
		{ DETAIL_BYTEDATA_INDEX_CHECK(offset, array.size()); }

		template < typename U, typename EnableIf<IsConvertible<U*, T*>::Value, bool>::ValueT = false >
		BasicByteData(const BasicByteArray<U>& array, size_t offset, size_t size)
			: _data(array.data() + offset), _size(size)
		{ DETAIL_BYTEDATA_INDEX_CHECK(offset + _size, array.size()); }

		template < typename U, size_t N >
		BasicByteData(U(&arr)[N])
			: _data(&arr[0]), _size(N)
		{ }

		BasicByteData(T* data, size_t size)
			: _data(data), _size(size)
		{ }

		BasicByteData(BasicByteData data, size_t offset)
			: _data(data._data + offset), _size(data._size - offset)
		{ DETAIL_BYTEDATA_INDEX_CHECK(offset, data._size); }

		BasicByteData(BasicByteData data, size_t offset, size_t size)
			: _data(data._data + offset), _size(size)
		{ DETAIL_BYTEDATA_INDEX_CHECK(offset + size, data._size); }

		T& operator [] (size_t index) const
		{
			STINGRAYKIT_CHECK(index < _size, IndexOutOfRangeException(index, _size));
			return _data[index];
		}

		T* data() const			{ return _data; }

		size_t size() const
		{ return _size; }

		bool empty() const
		{ return _size == 0; }

		iterator begin() const
		{ return Detail::ByteDataIteratorSelector<T>::CreateIterator(_data, _data, _data + _size); }

		iterator end() const
		{ return Detail::ByteDataIteratorSelector<T>::CreateIterator(_data + _size, _data, _data + _size); }

		reverse_iterator rbegin() const
		{ return reverse_iterator(end()); }

		reverse_iterator rend() const
		{ return reverse_iterator(begin()); }

		bool operator == (const BasicByteData& other) const
		{ return size() == other.size() && std::equal(data(), data() + size(), other.data()); }

		STINGRAYKIT_GENERATE_EQUALITY_OPERATORS_FROM_EQUAL(BasicByteData);

		bool operator < (const BasicByteData& other) const
		{ return std::lexicographical_compare(data(), data() + size(), other.data(), other.data() + other.size()); }

		STINGRAYKIT_GENERATE_RELATIONAL_OPERATORS_FROM_LESS(BasicByteData);
	};


	typedef BasicByteData<const u8>			ConstByteData;
	typedef BasicByteData<u8>				ByteData;

	typedef BasicByteArray<const u8>		ConstByteArray;
	typedef BasicByteArray<u8>				ByteArray;

	/** @} */

}

#undef DETAIL_BYTEDATA_INDEX_CHECK

#endif
