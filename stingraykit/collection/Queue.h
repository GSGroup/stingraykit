#ifndef STINGRAYKIT_COLLECTION_QUEUE_H
#define STINGRAYKIT_COLLECTION_QUEUE_H

// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/Range.h>

#include <deque>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template <
			typename T,
			template <class, class> class Container_ = std::deque,
			typename Allocator = std::allocator<T>
			>
	class Queue
	{
		typedef Container_<T, Allocator>								Container;

	public:
		typedef typename Container::value_type							value_type;
		typedef typename Container::size_type							size_type;
		typedef typename Container::difference_type						difference_type;
		typedef Allocator												allocator_type;
		typedef typename Allocator::reference							reference;
		typedef typename Allocator::const_reference						const_reference;
		typedef typename Allocator::pointer								pointer;
		typedef typename Allocator::const_pointer						const_pointer;
		typedef typename Container::iterator							iterator;
		typedef typename Container::const_iterator						const_iterator;
		typedef typename Container::reverse_iterator					reverse_iterator;
		typedef typename Container::const_reverse_iterator				const_reverse_iterator;

	private:
		Container		_container;

	public:
		explicit Queue(const Container& cont = Container())
			: _container(cont)
		{ }

		template < typename Range_ >
		explicit Queue(Range_ range)
		{ Range::Copy(range, std::back_inserter(_container)); }

		Queue(const Queue& other)
			: _container(other._container)
		{ }

		T& front()
		{
			STINGRAYKIT_CHECK(!empty(), "Queue is empty!");
			return _container.front();
		}

		const T& front() const
		{
			STINGRAYKIT_CHECK(!empty(), "Queue is empty!");
			return _container.front();
		}

		T& back()
		{
			STINGRAYKIT_CHECK(!empty(), "Queue is empty!");
			return _container.back();
		}

		const T& back() const
		{
			STINGRAYKIT_CHECK(!empty(), "Queue is empty!");
			return _container.back();
		}

		iterator begin()									{ return _container.begin(); }
		const_iterator begin() const						{ return _container.begin(); }
		iterator end()										{ return _container.end(); }
		const_iterator end() const							{ return _container.end(); }

		reverse_iterator rbegin()									{ return _container.rbegin(); }
		const_reverse_iterator rbegin() const						{ return _container.rbegin(); }
		reverse_iterator rend()										{ return _container.rend(); }
		const_reverse_iterator rend() const							{ return _container.rend(); }

		size_t size() const									{ return _container.size(); }
		bool empty() const									{ return _container.empty(); }
		void clear()										{ _container.clear(); }

		iterator erase(iterator pos)						{ return _container.erase(pos); }
		iterator erase(iterator first, iterator last)		{ return _container.erase(first, last); }

		void push(const T& value)
		{ _container.push_back(value); }

		template < typename Range_ >
		void push(Range_ range)
		{ Range::Copy(range, std::back_inserter(_container)); }

		optional<T> pop()
		{
			if (_container.empty())
				return null;

			T ret = _container.front();
			_container.erase(_container.begin());
			return ret;
		}
	};

	/** @} */

}

#endif
