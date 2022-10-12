#ifndef STINGRAYKIT_COLLECTION_COLLECTIONHELPERS_H
#define STINGRAYKIT_COLLECTION_COLLECTIONHELPERS_H

// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/exception.h>

namespace stingray
{

	template < class T, class Allocator = std::allocator<T> >
	class AllocatorValueHolder
	{
		STINGRAYKIT_NONCOPYABLE(AllocatorValueHolder);

		using AllocatorTraits = std::allocator_traits<Allocator>;

	private:
		Allocator*					_alloc;
		T*							_value;

	public:
		AllocatorValueHolder(AllocatorValueHolder&& other)
			: _alloc(other._alloc), _value(other.release())
		{ }

		~AllocatorValueHolder()
		{ reset(); }

		AllocatorValueHolder& operator = (AllocatorValueHolder&& other)
		{
			reset();
			_alloc = other._alloc;
			_value = other.release();
			return *this;
		}

		T* get() const							{ return _value; }
		T* operator -> () const					{ return STINGRAYKIT_REQUIRE_NOT_NULL(_value); }
		T& operator * () const					{ return *STINGRAYKIT_REQUIRE_NOT_NULL(_value); }

		T* release()
		{
			T* ptr = _value;
			_value = 0;
			return ptr;
		}

		void swap(AllocatorValueHolder& other)
		{
			std::swap(_alloc, other._alloc);
			std::swap(_value, other._value);
		}

		template < typename... Ts >
		static AllocatorValueHolder create(Allocator& alloc, Ts&&... args)
		{
			T* value = AllocatorTraits::allocate(alloc, 1);

			try
			{ AllocatorTraits::construct(alloc, value, std::forward<Ts>(args)...); }
			catch (...)
			{
				AllocatorTraits::deallocate(alloc, value, 1);
				throw;
			}

			return AllocatorValueHolder(alloc, value);
		}

	private:
		AllocatorValueHolder(Allocator& alloc, T* value)
			: _alloc(&alloc), _value(value)
		{ }

		void reset()
		{
			if (!_value)
				return;

			AllocatorTraits::destroy(*_alloc, _value);
			AllocatorTraits::deallocate(*_alloc, _value, 1);
			_value = 0;
		}
	};

}

#endif
