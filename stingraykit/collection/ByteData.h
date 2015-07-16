#ifndef STINGRAYKIT_COLLECTION_BYTEDATA_H
#define STINGRAYKIT_COLLECTION_BYTEDATA_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/iterator_base.h>
#include <stingraykit/shared_ptr.h>
#include <stingraykit/Dummy.h>
#include <stingraykit/MetaProgramming.h>
#include <stingraykit/TypeInfo.h>

#include <vector>
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

	public:
		typedef typename base::pointer													pointer;
		typedef typename base::reference												reference;
		typedef typename base::difference_type											difference_type;

	private:
		pointer _ptr;
		pointer _begin, _end;


	public:
		explicit ByteDataIterator(const pointer ptr, const pointer begin, const pointer end)
			: _ptr(ptr)
			, _begin(begin), _end(end)
		{ if (_ptr < _begin || _ptr > _end) STINGRAYKIT_THROW(IndexOutOfRangeException()); }

		reference dereference() const
		{
			if (_ptr < _begin || _ptr >= _end) STINGRAYKIT_THROW(IndexOutOfRangeException());
			return *_ptr;
		}
		bool equal(const ByteDataIterator &other) const						{ return _ptr == other._ptr; }
		void increment()													{ ++_ptr; }
		void decrement()													{ --_ptr; }
		void advance(difference_type n)										{ _ptr += n; }
		difference_type distance_to(const ByteDataIterator &other) const	{ return other._ptr - _ptr; }
	};


	namespace Detail
	{
#if !defined(PRODUCTION_BUILD)
		template<typename T>
		struct ByteDataIteratorSelector
		{
			typedef ByteDataIterator<T>			iterator;
			typedef ByteDataIterator<const T>	const_iterator;

			static inline iterator CreateIterator(T* ptr, T* begin, T* end)
			{ return iterator(ptr, begin, end); }
			static inline const_iterator CreateConstIterator(const T* ptr, const T* begin, const T* end)
			{ return const_iterator(ptr, begin, end); }
		};
#else
		template<typename T>
		struct ByteDataIteratorSelector
		{
			typedef T*			iterator;
			typedef const T*	const_iterator;

			static inline iterator CreateIterator(T* ptr, T* begin, T* end)
			{ (void)begin; (void)end; return iterator(ptr); }
			static inline const_iterator CreateConstIterator(const T* ptr, const T* begin, const T* end)
			{ (void)begin; (void)end; return const_iterator(ptr); }
		};
#endif
	}

	template < typename T >
	class BasicByteData;

	namespace ByteArrayUtils
	{

		STINGRAYKIT_DECLARE_METHOD_CHECK(begin);
		STINGRAYKIT_DECLARE_METHOD_CHECK(end);

		template < typename T >
		struct HasBeginEndMethods
		{
			static const bool Value = HasMethod_begin<T>::Value && HasMethod_end<T>::Value;
		};

	}

	/**
	 * @brief An object that retains shared ownership of an array of bytes
	 */
	template < typename T >
	class BasicByteArray
	{
		template < typename U >
		friend class BasicByteArray;

		typedef typename Deconst<T>::ValueT NonConstType;
		typedef typename
			If<
				IsConst<T>::Value,
				const std::vector<NonConstType>,
				std::vector<T>
			>::ValueT	CollectionType;

		typedef shared_ptr<CollectionType>		CollectionTypePtr;

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
			: _data(new CollectionType()), _offset(0), _sizeLimit(NoSizeLimit)
		{ }

		explicit BasicByteArray(size_t size)
			: _data(new CollectionType(size)), _offset(0), _sizeLimit(NoSizeLimit)
		{ }

		BasicByteArray(const T* data, size_t size)
			: _data(new CollectionType(size)), _offset(0), _sizeLimit(NoSizeLimit)
		{ std::copy(data, data + size, _data->begin()); }

		template < typename U >
		BasicByteArray(const U& range, typename EnableIf<ByteArrayUtils::HasBeginEndMethods<U>::Value, Dummy>::ValueT* dummy = 0)
			: _data(new CollectionType(range.begin(), range.end())), _offset(0), _sizeLimit(NoSizeLimit)
		{ }

		template < typename InputIterator >
		BasicByteArray(InputIterator first, InputIterator last)
			: _data(new CollectionType(first, last)), _offset(0), _sizeLimit(NoSizeLimit)
		{ }

		template < typename U >
		BasicByteArray(const BasicByteArray<U>& other)
			: _data(other.GetData()), _offset(other.GetOffset()), _sizeLimit(other._sizeLimit)
		{ }

		template < typename U >
		BasicByteArray(const BasicByteArray<U>& other, size_t offset)
			: _data(other.GetData()), _offset(other.GetOffset() + offset), _sizeLimit(other._sizeLimit == NoSizeLimit ? NoSizeLimit : other._sizeLimit - offset)
		{ STINGRAYKIT_CHECK(_data->size() >= _offset, ArgumentException("offset")); }

		template < typename U >
		BasicByteArray(const BasicByteArray<U>& other, size_t offset, size_t sizeLimit)
			: _data(other.GetData()), _offset(other.GetOffset() + offset), _sizeLimit(sizeLimit)
		{
			STINGRAYKIT_CHECK(_data->size() >= _offset, ArgumentException("offset"));
			STINGRAYKIT_CHECK((_sizeLimit == NoSizeLimit || _sizeLimit + offset <= _data->size()) && (_sizeLimit + offset <= other._sizeLimit), ArgumentException("sizeLimit"));
		}

		inline size_t GetOffset() const { return _offset; }
		inline CollectionTypePtr GetData() const { return _data; }

		inline BasicByteData<T> GetByteData() const { return BasicByteData<T>(*this); }

		inline bool CheckIterator(const const_iterator& it) const	{ return it - begin() >= 0 && end() - it > 0; }

		void RequireSize(size_t size)
		{
			if (_data->size() < size + _offset)
			{
				STINGRAYKIT_CHECK(_sizeLimit == NoSizeLimit, NotImplementedException());
				_data->resize(size + _offset);
			}
		}

		inline T& operator[](size_t index) const
		{
			if (index >= size())
				STINGRAYKIT_THROW(IndexOutOfRangeException(index, size()));
			return (*_data)[index + _offset];
		}

		inline size_t size() const
		{ return std::min(_data->size() - _offset, _sizeLimit); }

		inline bool empty() const
		{ return ((int)(_data->size()) - (int)_offset) <= 0; }

		template < typename InputIterator >
		void append(InputIterator first, InputIterator last)
		{
			STINGRAYKIT_CHECK(_sizeLimit == NoSizeLimit, NotImplementedException());
			_data->insert(_data->end(), first, last);
		}

		void reserve(size_t n)
		{ _data->reserve(_offset + n); }

		template <typename U>
		void swap(BasicByteArray<U>& other)
		{
			_data.swap(other._data);
			std::swap(_offset, other._offset);
			std::swap(_sizeLimit, other._sizeLimit);
		}

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

		inline T* data() const						{ return (_data->empty() ? NULL : &(*_data)[0]) + _offset; }

		template<typename ObjectOStream>
		void Serialize(ObjectOStream & ar) const
		{
			//why would we store whole array anyway?
			//ar.Serialize("o", _offset); ar.Serialize("d", *_data);
			std::vector<u8> data_proxy(data(), data() + size());//fixme: later
			ar.Serialize("d", data_proxy);
		}

		template<typename ObjectIStream>
		void Deserialize(ObjectIStream & ar)		{ BasicByteArray data; ar.Deserialize("o", data._offset, 0); ar.Deserialize("d", const_cast<std::vector<NonConstType> &>(*data._data)); *this = data; }

		std::string ToString() const				{ return "BasicByteArray<" + TypeInfo(typeid(T)).GetName() + "> { size : " + stingray::ToString(_data->size()) + " }"; }

		bool operator == (const BasicByteArray& other) const
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

		BasicByteData() :
			_data(), _size()
		{ }

		template < typename U >
		BasicByteData(BasicByteData<U> data) :
			_data(data.size() == 0 ? NULL : &data[0]), _size(data.size())
		{ }

		template < typename U >
		BasicByteData(std::vector<U>& data) :
			_data(data.empty() ? NULL : &data[0]), _size(data.size())
		{ }

		template < typename U >
		BasicByteData(const std::vector<U>& data) :
			_data(data.empty() ? NULL : &data[0]), _size(data.size())
		{ }

		template < typename U >
		explicit BasicByteData(BasicByteArray<U>& array) : //disallow storing anonymous copies without explicit intention to do so
			_data(array.data()), _size(array.size())
		{ }

		template < typename U >
		explicit BasicByteData(const BasicByteArray<U>& array) : //disallow storing anonymous copies without explicit intention to do so
			_data(array.data()), _size(array.size())
		{ }

		template < typename U >
		BasicByteData(BasicByteArray<U>& array, size_t offset) :
			_data(array.data() + offset), _size(array.size() - offset)
		{ DETAIL_BYTEDATA_INDEX_CHECK(offset, array.size()); }

		template < typename U >
		BasicByteData(const BasicByteArray<U>& array, size_t offset) :
			_data(array.data() + offset), _size(array.size() - offset)
		{ DETAIL_BYTEDATA_INDEX_CHECK(offset, array.size()); }

		template < typename U >
		BasicByteData(BasicByteArray<U>& array, size_t offset, size_t size) :
			_data(array.data() + offset), _size(size)
		{ DETAIL_BYTEDATA_INDEX_CHECK(offset + _size, array.size()); }

		template < typename U >
		BasicByteData(const BasicByteArray<U>& array, size_t offset, size_t size) :
			_data(array.data() + offset), _size(size)
		{ DETAIL_BYTEDATA_INDEX_CHECK(offset + _size, array.size()); }

		template<typename U, size_t N>
		BasicByteData(U(&arr)[N]) :
			_data(&arr[0]), _size(N)
		{ }

		BasicByteData(T* data, size_t size) :
			_data(data), _size(size)
		{ }

		BasicByteData(BasicByteData data, size_t offset) :
			_data(data._data + offset), _size(data._size - offset)
		{ DETAIL_BYTEDATA_INDEX_CHECK(offset, data._size); }

		BasicByteData(BasicByteData data, size_t offset, size_t size) :
			_data(data._data + offset), _size(size)
		{ DETAIL_BYTEDATA_INDEX_CHECK(offset + size, data._size); }

		inline BasicByteData<T> GetByteData() const { return BasicByteData<T>(*this); } //made compatible with array for template classes

		inline bool CheckIterator(const const_iterator& it) const	{ return std::distance(it, end()) > 0; }

		inline T& operator[](size_t index) const
		{
			if (index >= _size)
				STINGRAYKIT_THROW(IndexOutOfRangeException(index, _size));
			return _data[index];
		}

		inline T* data() const			{ return _data; }

		inline size_t size() const
		{ return _size; }

		inline bool empty() const
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
