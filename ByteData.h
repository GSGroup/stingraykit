#ifndef __GS_DVRLIB_TOOLKIT_BYTEDATA_H__
#define __GS_DVRLIB_TOOLKIT_BYTEDATA_H__

#include <vector>

#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/fatal.h>
#include <stingray/toolkit/iterator_base.h>
#include <stingray/toolkit/shared_ptr.h>
#include <stingray/toolkit/toolkit.h>

#define DETAIL_BYTEDATA_INDEX_CHECK(...) TOOLKIT_INDEX_CHECK(__VA_ARGS__)


namespace stingray
{
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

#define	ITERATOR_CHECK_BOUNDS			do { if (_ptr < _begin || _ptr > _end) TOOLKIT_FATAL("Iterator is out of bounds!"); } while (false)
#define ITERATOR_CHECK_BOUNDS_STRICT	do { if (_ptr < _begin || _ptr >= _end) TOOLKIT_FATAL("Iterator is out of bounds!"); } while (false)

	public:
		explicit ByteDataIterator(const pointer ptr, const pointer begin, const pointer end)
			: _ptr(ptr)
			, _begin(begin), _end(end)
		{ (void)begin; (void)end; ITERATOR_CHECK_BOUNDS; }

		reference dereference() const										{ ITERATOR_CHECK_BOUNDS_STRICT; return *_ptr; }
		bool equal(const ByteDataIterator &other) const						{ return _ptr == other._ptr; }
		void increment()													{ ++_ptr; ITERATOR_CHECK_BOUNDS; }
		void decrement()													{ --_ptr; ITERATOR_CHECK_BOUNDS; }
		void advance(difference_type n)										{ _ptr += n; ITERATOR_CHECK_BOUNDS; }
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

			static FORCE_INLINE iterator CreateIterator(T* ptr, T* begin, T* end)
			{ return iterator(ptr, begin, end); }
			static FORCE_INLINE const_iterator CreateConstIterator(const T* ptr, const T* begin, const T* end)
			{ return const_iterator(ptr, begin, end); }
		};
#else
		template<typename T>
		struct ByteDataIteratorSelector
		{
			typedef T*			iterator;
			typedef const T*	const_iterator;

			static FORCE_INLINE iterator CreateIterator(T* ptr, T* begin, T* end)
			{ (void)begin; (void)end; return iterator(ptr); }
			static FORCE_INLINE const_iterator CreateConstIterator(const T* ptr, const T* begin, const T* end)
			{ (void)begin; (void)end; return const_iterator(ptr); }
		};
#endif
	}


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
		typedef typename Detail::ByteDataIteratorSelector<T>::iterator			iterator;
		typedef typename Detail::ByteDataIteratorSelector<T>::const_iterator	const_iterator;

		BasicByteArray()
			: _data(new CollectionType()), _offset(0), _sizeLimit(NoSizeLimit)
		{ }

		explicit BasicByteArray(size_t size)
			: _data(new CollectionType(size)), _offset(0), _sizeLimit(NoSizeLimit)
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
		{ TOOLKIT_CHECK(_data->size() >= _offset, ArgumentException("offset")); }

		template < typename U >
		BasicByteArray(const BasicByteArray<U>& other, size_t offset, size_t sizeLimit)
			: _data(other.GetData()), _offset(other.GetOffset() + offset), _sizeLimit(sizeLimit)
		{
			TOOLKIT_CHECK(_data->size() >= _offset, ArgumentException("offset"));
			TOOLKIT_CHECK((_sizeLimit == NoSizeLimit || _sizeLimit + offset <= _data->size()) && (_sizeLimit + offset <= other._sizeLimit), ArgumentException("sizeLimit"));
		}

		inline size_t GetOffset() const { return _offset; }
		inline CollectionTypePtr GetData() const { return _data; }

		void RequireSize(size_t size)
		{
			if (_data->size() < size + _offset)
			{
				TOOLKIT_CHECK(_sizeLimit == NoSizeLimit, NotImplementedException());
				_data->resize(size + _offset);
			}
		}

		FORCE_INLINE T& operator[](size_t index) const
		{
			DETAIL_BYTEDATA_INDEX_CHECK(index < size());
			return (*_data)[index + _offset];
		}

		FORCE_INLINE size_t size() const
		{ return std::min(_data->size() - _offset, _sizeLimit); }

		FORCE_INLINE bool empty() const
		{ return ((int)(_data->size()) - (int)_offset) <= 0; }

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

		inline T* data()				{ return (_data->empty() ? NULL : &(*_data)[0]) + _offset; }
		inline const T* data() const	{ return (_data->empty() ? NULL : &(*_data)[0]) + _offset; }

		template<typename ObjectOStream>
		void Serialize(ObjectOStream & ar) const	{ ar.Serialize("o", _offset); ar.Serialize("d", *_data);}

		template<typename ObjectIStream>
		void Deserialize(ObjectIStream & ar)		{ BasicByteArray data; ar.Deserialize("o", _offset, 0); ar.Deserialize("d", const_cast<std::vector<NonConstType> &>(*data._data)); *this = data; }
	};

	template < typename T >
	class BasicByteData
	{
	private:
		T*		_data;
		size_t	_size;

	public:
		typedef typename Detail::ByteDataIteratorSelector<T>::iterator			iterator;
		typedef typename Detail::ByteDataIteratorSelector<T>::const_iterator	const_iterator;

		template < typename U >
		BasicByteData(const BasicByteData<U>& data)
			: _data(data.size() == 0 ? NULL : &data[0]), _size(data.size())
		{ }

		template < typename U >
		BasicByteData(std::vector<U>& data)
			: _data(data.empty() ? NULL : &data[0]), _size(data.size())
		{ }

		template < typename U >
		BasicByteData(const std::vector<U>& data)
			: _data(data.empty() ? NULL : &data[0]), _size(data.size())
		{ }

		template < typename U >
		BasicByteData(BasicByteArray<U>& array)
			: _data(array.data()), _size(array.size())
		{ }

		template < typename U >
		BasicByteData(const BasicByteArray<U>& array)
			: _data(array.data()), _size(array.size())
		{ }

		BasicByteData(T* data, size_t size)
			: _data(data), _size(size)
		{ }

		BasicByteData(const BasicByteData& data, size_t offset)
			: _data(data._data + offset), _size(data._size - offset)
		{ DETAIL_BYTEDATA_INDEX_CHECK(offset <= data._size); }

		BasicByteData(const BasicByteData& data, size_t offset, size_t size)
			: _data(data._data + offset), _size(size)
		{ DETAIL_BYTEDATA_INDEX_CHECK(offset + size <= data._size); }

		FORCE_INLINE T& operator[](size_t index) const
		{
			DETAIL_BYTEDATA_INDEX_CHECK(index < _size);
			return _data[index];
		}

		inline T* data()				{ return _data; }
		inline const T* data() const	{ return _data; }

		FORCE_INLINE size_t size() const
		{ return _size; }

		FORCE_INLINE bool empty() const
		{ return _size == 0; }

		iterator begin() const
		{ return Detail::ByteDataIteratorSelector<T>::CreateIterator(_data, _data, _data + _size); }

		iterator end() const
		{ return Detail::ByteDataIteratorSelector<T>::CreateIterator(_data + _size, _data, _data + _size); }
	};



	typedef BasicByteData<const u8>			ConstByteData;
	typedef BasicByteData<u8>				ByteData;

	typedef BasicByteArray<const u8>		ConstByteArray;
	typedef BasicByteArray<u8>				ByteArray;

}

#undef DETAIL_BYTEDATA_INDEX_CHECK

#endif
