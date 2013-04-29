#ifndef STINGRAY_TOOLKIT_INPLACE_VECTOR_H
#define STINGRAY_TOOLKIT_INPLACE_VECTOR_H

#include <stingray/toolkit/aligned_storage.h>
#include <stingray/toolkit/array.h>
#include <vector>

namespace stingray
{

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

		template<typename ValueType>
		struct base_iterator
		{
			typedef	ValueType							value_type;
			typedef ptrdiff_t							difference_type;
			typedef std::random_access_iterator_tag		iterator_category;
			typedef value_type *						pointer;
			typedef value_type &						reference;

		private:
			typedef inplace_vector						owner_type;

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
			inline pointer operator -> () const	{ return &_owner.at(_pos); }
		};

	public:
		typedef base_iterator<T>			iterator;
		typedef base_iterator<const T>		const_iterator;

		inline inplace_vector(): _staticStorageSize(0) {}
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

		inline size_t size() const { return _staticStorageSize + _dynamicStorage.size(); }

		inline const_iterator begin() const
		{ return iterator(*this, 0); }
		inline iterator begin()
		{ return iterator(*this, 0); }

		inline const_iterator end() const
		{ return iterator(*this, size()); }
		inline iterator end()
		{ return iterator(*this, size()); }

	};

}

#endif
