#ifndef STINGRAYKIT_COLLECTION_INPLACE_VECTOR_H
#define STINGRAYKIT_COLLECTION_INPLACE_VECTOR_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
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
		static_assert(InplaceCapacity_ != 0, "Invalid inplace capacity");

	private:
		template < typename OwnerType, typename ValueType >
		class Iterator : public iterator_base<Iterator<OwnerType, ValueType>, ValueType, std::random_access_iterator_tag>
		{
			using base = iterator_base<Iterator<OwnerType, ValueType>, ValueType, std::random_access_iterator_tag>;

			template < typename OtherOwnerType, typename OtherValueType >
			friend class Iterator;

		private:
			OwnerType&				_owner;
			size_t					_offset;

		public:
			Iterator(OwnerType& owner, size_t offset)
				: _owner(owner), _offset(offset)
			{ }

			template < typename OtherOwnerType, typename OtherValueType >
			Iterator(const Iterator<OtherOwnerType, OtherValueType>& other)
				: _owner(other._owner), _offset(other._offset)
			{ }

			typename base::reference dereference() const
			{ return _owner.at(_offset); }

			bool equal(const Iterator& other) const
			{ return _offset == other._offset; }

			void increment()
			{ ++_offset; }

			void decrement()
			{ --_offset; }

			void advance(typename base::difference_type diff)
			{ _offset += diff; }

			typename base::difference_type distance_to(const Iterator& other) const
			{ return other._offset - _offset; }
		};

	public:
		using value_type = T;
		using difference_type = ptrdiff_t;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using iterator = Iterator<inplace_vector, value_type>;
		using const_iterator = Iterator<const inplace_vector, const value_type>;

	public:
		static const size_t InplaceCapacity = InplaceCapacity_;

	private:
		size_t									_staticStorageSize;
		array<StorageFor<T>, InplaceCapacity>	_staticStorage;
		std::vector<T>							_dynamicStorage;

	public:
		inplace_vector()
			: _staticStorageSize(0)
		{ }

		~inplace_vector()
		{
			for (size_t index = 0; index < _staticStorageSize; ++index)
				_staticStorage[index].Dtor();
		}

		template < typename InputIterator >
		void assign(InputIterator first, InputIterator last)
		{
			clear();
			reserve(std::distance(first, last));
			std::copy(first, last, std::back_inserter(*this));
		}

		T& at(size_t index)
		{ return index < _staticStorageSize ? _staticStorage[index].Ref() : _dynamicStorage.at(index - _staticStorageSize); }

		const T& at(size_t index) const
		{ return index < _staticStorageSize ? _staticStorage[index].Ref() : _dynamicStorage.at(index - _staticStorageSize); }

		T& operator [] (size_t index)
		{ return at(index); }

		const T& operator [] (size_t index) const
		{ return at(index); }

		iterator begin()						{ return iterator(*this, 0); }
		const_iterator begin() const			{ return const_iterator(*this, 0); }

		iterator end()							{ return iterator(*this, size()); }
		const_iterator end() const				{ return const_iterator(*this, size()); }

		bool empty() const						{ return size() == 0; }
		size_t size() const						{ return _staticStorageSize + _dynamicStorage.size(); }

		void reserve(size_t capacity)
		{
			if (capacity > InplaceCapacity)
				_dynamicStorage.reserve(capacity - InplaceCapacity);
			else
				_dynamicStorage.reserve(0);
		}

		size_t capacity() const					{ return InplaceCapacity + _dynamicStorage.capacity(); }

		void clear()
		{
			_dynamicStorage.clear();
			for (size_t index = 0; index < _staticStorageSize; ++index)
				_staticStorage[index].Dtor();
			_staticStorageSize = 0;
		}

		void push_back(const T& value)
		{
			if (_staticStorageSize < InplaceCapacity)
				_staticStorage[_staticStorageSize++].Ctor(value);
			else
				_dynamicStorage.push_back(value);
		}
	};

	/** @} */

}

#endif
