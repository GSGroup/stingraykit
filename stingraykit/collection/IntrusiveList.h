#ifndef STINGRAYKIT_COLLECTION_INTRUSIVELIST_H
#define STINGRAYKIT_COLLECTION_INTRUSIVELIST_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/Types.h>
#include <stingraykit/assert.h>
#include <stingraykit/collection/iterator_base.h>
#include <stingraykit/exception.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template <typename T>
	class IntrusiveList;


	class IntrusiveListNodeData
	{
		template <typename U>
		friend class IntrusiveList;

		IntrusiveListNodeData	*_prev, *_next;

	protected:
		inline IntrusiveListNodeData() :
			_prev(this), _next(this)
		{ }

		~IntrusiveListNodeData()
		{ }

	private:
		inline void unlink()
		{
			_next->_prev = _prev;
			_prev->_next = _next;
			_prev = _next = this;
		}

		inline bool unlinked() const { return _next == this; }

		inline void insert_before(IntrusiveListNodeData *node)
		{
			_prev = node->_prev;
			_next = node;

			_next->_prev = _prev->_next = this;
		}

		inline void insert_after(IntrusiveListNodeData *node)
		{
			_prev = node;
			_next = node->_next;

			_next->_prev = _prev->_next = this;
		}
	};


	template <typename T>
	class IntrusiveList
	{
		STINGRAYKIT_NONCOPYABLE(IntrusiveList);

	public:
		typedef T						value_type;
		typedef IntrusiveListNodeData	NodeDataType;

	public:
		class iterator : public iterator_base<iterator, T, std::bidirectional_iterator_tag>
		{
			typedef iterator_base<iterator, T, std::bidirectional_iterator_tag>	base;

		private:
			IntrusiveListNodeData*	_current;

		public:
			iterator(IntrusiveListNodeData* current) : _current(current)
			{ }

			typename base::reference dereference() const	{ return *static_cast<T*>(_current); }
			bool equal(const iterator &other) const			{ return _current == other._current; }
			void increment()								{ _current = get_next(_current); }
			void decrement()								{ _current = get_prev(_current); }
		};

		class const_iterator : public iterator_base<const_iterator, const T, std::bidirectional_iterator_tag>
		{
			typedef iterator_base<const_iterator, const T, std::bidirectional_iterator_tag>	base;

		private:
			const IntrusiveListNodeData*	_current;

		public:
			const_iterator(const IntrusiveListNodeData* current) : _current(current)
			{ }

			typename base::reference dereference() const	{ return *static_cast<const T*>(_current); }
			bool equal(const const_iterator &other) const	{ return _current == other._current; }
			void increment()								{ _current = get_next(_current); }
			void decrement()								{ _current = get_prev(_current); }
		};

	private:
		IntrusiveListNodeData	_root;

		static NodeDataType* get_next(const NodeDataType* n)	{ return n->_next; }
		static NodeDataType* get_prev(const NodeDataType* n)	{ return n->_prev; }

	public:
		IntrusiveList() : _root()
		{ }

		~IntrusiveList()
		{ STINGRAYKIT_ASSERT(empty()); }

		iterator begin()				{ return iterator(_root._next); }
		iterator end()					{ return iterator(&_root); }

		// Sorry. =)
		const_iterator begin() const	{ return const_iterator(_root._next); }
		const_iterator end() const		{ return const_iterator(&_root); }

		inline bool empty() const		{ return _root.unlinked(); }
		size_t size() const				{ return std::distance(begin(), end()); }

		void push_back(T& value)		{ value.insert_before(&_root); }
		void erase(T& value)			{ value.unlink(); }
	};

	/** @} */


}



#endif
