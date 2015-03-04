#ifndef STINGRAYKIT_COLLECTION_INPLACE_VECTOR_H
#define STINGRAYKIT_COLLECTION_INPLACE_VECTOR_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/aligned_storage.h>
#include <stingraykit/collection/array.h>
#include <vector>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template<typename T, size_t InplaceCapacity_>
	class inplace_vector
	{
	public:
		static const size_t InplaceCapacity = InplaceCapacity_;

		typedef	T									value_type;
		typedef ptrdiff_t							difference_type;
		typedef value_type *						pointer;
		typedef value_type &						reference;
		typedef const value_type *					const_pointer;
		typedef const value_type &					const_reference;

	private:
		size_t									_staticStorageSize;
		array<StorageFor<T>, InplaceCapacity>	_staticStorage;
		std::vector<T>							_dynamicStorage;

		template<typename OwnerType, typename ValueType>
		class base_iterator
		{
			typedef OwnerType							owner_type;

		public:
			typedef	ValueType							value_type;
			typedef ptrdiff_t							difference_type;
			typedef std::random_access_iterator_tag		iterator_category;
			typedef value_type *						pointer;
			typedef value_type &						reference;


			owner_type &								_owner;
			size_t										_pos;

		public:
			base_iterator(owner_type& owner, size_t pos): _owner(owner), _pos(pos) {}

			inline bool operator == (const base_iterator & other) const
			{ return _pos == other._pos; }
			inline bool operator != (const base_iterator & other) const
			{ return !((*this) == other); }
			inline bool operator < (const base_iterator & other) const
			{ return _pos < other._pos; }

			inline base_iterator& operator ++ ()
			{ ++_pos; return *this; }
			inline base_iterator operator ++ (int)
			{ base_iterator prev(*this); ++(*this); return prev; }
			inline base_iterator& operator += (difference_type d)
			{ _pos += d; return *this; }
			inline base_iterator operator + (difference_type d) const
			{ return base_iterator(_owner, _pos + d); }

			inline base_iterator& operator -- ()
			{ --_pos; return *this; }
			inline base_iterator operator -- (int)
			{ base_iterator prev(*this); --(*this); return prev; }
			inline base_iterator& operator -= (difference_type d)
			{ _pos -= d; return *this; }
			inline base_iterator operator - (difference_type d) const
			{ return base_iterator(_owner, _pos - d); }

			inline difference_type operator -(const base_iterator & other) const
			{ return _pos - other._pos; }

			inline reference operator [] (difference_type d) const
			{ return _owner.at(_pos + d); }

			inline reference operator * () const	{ return _owner.at(_pos); }
			inline pointer operator -> () const		{ return &_owner.at(_pos); }
		};

	public:
		typedef base_iterator<inplace_vector, value_type>				iterator;
		typedef base_iterator<const inplace_vector, const value_type>	const_iterator;

		inline inplace_vector(): _staticStorageSize(0) {}

		void clear()
		{
			_dynamicStorage.clear();
			for(size_t i = 0; i < _staticStorageSize; ++i) _staticStorage[i].Dtor();
			_staticStorageSize = 0;
		}

		inline ~inplace_vector()
		{
			for(size_t i = 0; i < _staticStorageSize; ++i) _staticStorage[i].Dtor();
		}

		inline T & at(size_t index)
		{ return (index < _staticStorageSize)? _staticStorage[index].Ref(): _dynamicStorage.at(index - _staticStorageSize); }

		inline const T & at(size_t index) const
		{ return(index < _staticStorageSize)? _staticStorage[index].Ref(): _dynamicStorage.at(index - _staticStorageSize); }

		inline T & operator [] (size_t index)
		{ return at(index); }

		inline const T & operator [] (size_t index) const
		{ return at(index); }

		inline void push_back(const T& value)
		{
			if (_staticStorageSize < InplaceCapacity)
			{
				_staticStorage[_staticStorageSize++].Ctor(value);
			}
			else
				_dynamicStorage.push_back(value);
		}

		template<typename assign_iterator_type>
		void assign(assign_iterator_type begin, assign_iterator_type end)
		{
			clear();
			reserve(std::distance(begin, end));
			std::copy(begin, end, std::back_inserter(*this));
		}

		inline size_t size() const { return _staticStorageSize + _dynamicStorage.size(); }

		void reserve(size_t capacity)
		{
			if (capacity > InplaceCapacity)
				_dynamicStorage.reserve(capacity - InplaceCapacity);
			else
				_dynamicStorage.reserve(0);
		}

		inline const_iterator begin() const
		{ return const_iterator(*this, 0); }
		inline iterator begin()
		{ return iterator(*this, 0); }

		inline const_iterator end() const
		{ return const_iterator(*this, size()); }
		inline iterator end()
		{ return iterator(*this, size()); }

	};

	/** @} */

}

#endif
