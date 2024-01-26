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

	public:
		using value_type = T;

	private:
		template < bool Const >
		class Iterator : public iterator_base<Iterator<Const>, typename If<Const, const value_type, value_type>::ValueT, std::random_access_iterator_tag>
		{
			using base = iterator_base<Iterator<Const>, typename If<Const, const value_type, value_type>::ValueT, std::random_access_iterator_tag>;

			using OwnerType = typename If<Const, const inplace_vector, inplace_vector>::ValueT;

			template < bool Const_ >
			friend class Iterator;

		private:
			OwnerType&				_owner;
			size_t					_offset;

		public:
			Iterator(OwnerType& owner, size_t offset)
				: _owner(owner), _offset(offset)
			{ }

			template < bool Const_, typename EnableIf<Const && !Const_, int>::ValueT = 0 >
			Iterator(const Iterator<Const_>& other)
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
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using iterator = Iterator<false>;
		using const_iterator = Iterator<true>;

	public:
		static const size_t InplaceCapacity = InplaceCapacity_;

	private:
		size_t												_staticStorageSize;
		array<StorageFor<value_type>, InplaceCapacity>		_staticStorage;
		std::vector<value_type>								_dynamicStorage;

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

		reference at(size_t index)
		{
			Detail::ArrayCheckRange(index, size());
			return index < _staticStorageSize ? _staticStorage[index].Ref() : _dynamicStorage[index - _staticStorageSize];
		}

		const_reference at(size_t index) const
		{
			Detail::ArrayCheckRange(index, size());
			return index < _staticStorageSize ? _staticStorage[index].Ref() : _dynamicStorage[index - _staticStorageSize];
		}

		reference operator [] (size_t index)
		{ return at(index); }

		const_reference operator [] (size_t index) const
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

		void push_back(const_reference value)
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
