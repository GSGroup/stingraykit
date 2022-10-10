#ifndef STINGRAYKIT_COLLECTION_INPLACE_VECTOR_H
#define STINGRAYKIT_COLLECTION_INPLACE_VECTOR_H

// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/array.h>
#include <stingraykit/collection/iterator_base.h>
#include <stingraykit/aligned_storage.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename T, size_t InplaceCapacity_ >
	class inplace_vector
	{
	public:
		static const size_t InplaceCapacity = InplaceCapacity_;

		typedef	T									value_type;
		typedef ptrdiff_t							difference_type;
		typedef value_type*							pointer;
		typedef value_type&							reference;
		typedef const value_type*					const_pointer;
		typedef const value_type&					const_reference;

	private:
		size_t									_staticStorageSize;
		array<StorageFor<T>, InplaceCapacity>	_staticStorage;
		std::vector<T>							_dynamicStorage;

	public:
		template < typename OwnerType, typename ValueType >
		class inplace_vector_iterator : public iterator_base<inplace_vector_iterator<OwnerType, ValueType>, ValueType, std::random_access_iterator_tag>
		{
			typedef iterator_base<inplace_vector_iterator<OwnerType, ValueType>, ValueType, std::random_access_iterator_tag> base;

			template < typename OtherOwnerType, typename OtherValueType >
			friend class inplace_vector_iterator;

		private:
			OwnerType&				_owner;
			size_t					_offset;

		public:
			inplace_vector_iterator(OwnerType& owner, size_t offset)
				: _owner(owner), _offset(offset)
			{ }

			template < typename OtherOwnerType, typename OtherValueType >
			inplace_vector_iterator(const inplace_vector_iterator<OtherOwnerType, OtherValueType>& other)
				: _owner(other._owner), _offset(other._offset)
			{ }

			typename base::reference dereference() const
			{ return _owner.at(_offset); }

			bool equal(const inplace_vector_iterator& other) const
			{ return _offset == other._offset; }

			void increment()
			{ ++_offset; }
			void decrement()
			{ --_offset; }
			void advance(typename base::difference_type diff)
			{ _offset += diff; }

			typename base::difference_type distance_to(const inplace_vector_iterator& other) const
			{ return other._offset - _offset; }
		};

		typedef inplace_vector_iterator<inplace_vector, value_type>					iterator;
		typedef inplace_vector_iterator<const inplace_vector, const value_type>		const_iterator;

		inplace_vector()
			: _staticStorageSize(0)
		{ static_assert(InplaceCapacity_ != 0, "Invalid inplace capacity"); }

		size_t capacity() const
		{ return InplaceCapacity + _dynamicStorage.capacity(); }

		void clear()
		{
			_dynamicStorage.clear();
			for (size_t i = 0; i < _staticStorageSize; ++i)
				_staticStorage[i].Dtor();
			_staticStorageSize = 0;
		}

		~inplace_vector()
		{
			for (size_t i = 0; i < _staticStorageSize; ++i)
				_staticStorage[i].Dtor();
		}

		T& at(size_t index)
		{ return (index < _staticStorageSize) ? _staticStorage[index].Ref() : _dynamicStorage.at(index - _staticStorageSize); }

		const T& at(size_t index) const
		{ return (index < _staticStorageSize) ? _staticStorage[index].Ref() : _dynamicStorage.at(index - _staticStorageSize); }

		T& operator [] (size_t index)
		{ return at(index); }

		const T& operator [] (size_t index) const
		{ return at(index); }

		void push_back(const T& value)
		{
			if (_staticStorageSize < InplaceCapacity)
				_staticStorage[_staticStorageSize++].Ctor(value);
			else
				_dynamicStorage.push_back(value);
		}

		template < typename assign_iterator_type >
		void assign(assign_iterator_type begin, assign_iterator_type end)
		{
			clear();
			reserve(std::distance(begin, end));
			std::copy(begin, end, std::back_inserter(*this));
		}

		size_t size() const { return _staticStorageSize + _dynamicStorage.size(); }
		bool empty() const { return size() == 0; }

		void reserve(size_t capacity)
		{
			if (capacity > InplaceCapacity)
				_dynamicStorage.reserve(capacity - InplaceCapacity);
			else
				_dynamicStorage.reserve(0);
		}

		const_iterator begin() const
		{ return const_iterator(*this, 0); }
		iterator begin()
		{ return iterator(*this, 0); }

		const_iterator end() const
		{ return const_iterator(*this, size()); }
		iterator end()
		{ return iterator(*this, size()); }

	};

	/** @} */

}

#endif
