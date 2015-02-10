#ifndef STINGRAYKIT_COLLECTION_INTRUSIVELIST_H
#define STINGRAYKIT_COLLECTION_INTRUSIVELIST_H

#include <stingraykit/Types.h>
#include <stingraykit/exception.h>
#include <stingraykit/collection/iterator_base.h>

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
		inline IntrusiveListNodeData()
			: _prev(this), _next(this)
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


	template < typename T >
	class IntrusiveListNode : public IntrusiveListNodeData
	{
	private:
		T	_data;

	public:
		IntrusiveListNode(const T& data) : _data(data)
		{ }

		operator T& ()				{ return _data; }
		operator const T& () const	{ return _data; }
	};


	template <typename T>
	class IntrusiveList
	{
		STINGRAYKIT_NONCOPYABLE(IntrusiveList);

	public:
		typedef T						value_type;
		typedef IntrusiveListNodeData	NodeDataType;
		typedef IntrusiveListNode<T>	NodeType;

	public:
		class iterator : public iterator_base<iterator, const T, std::bidirectional_iterator_tag>
		{
			typedef iterator_base<iterator, const T, std::bidirectional_iterator_tag>	base;

		private:
			const IntrusiveListNodeData*	_current;

		public:
			iterator(const IntrusiveListNodeData* current) : _current(current)
			{ }

			typename base::reference dereference() const	{ return *static_cast<const NodeType* >(_current); }
			bool equal(const iterator &other) const			{ return _current == other._current; }
			void increment()								{ _current = get_next(_current); }
			void decrement()								{ _current = get_prev(_current); }
		};
		typedef iterator const_iterator;

	private:
		IntrusiveListNodeData	_root;

		static NodeDataType* get_next(const NodeDataType* n)	{ return n->_next; }
		static NodeDataType* get_prev(const NodeDataType* n)	{ return n->_prev; }

	public:
		IntrusiveList() : _root()
		{ }

		~IntrusiveList()
		{ STINGRAYKIT_ASSERT(empty()); }

		iterator begin()			{ return iterator(_root._next); }
		iterator end()				{ return iterator(&_root); }

		// Sorry. =)
		iterator begin() const		{ return iterator(_root._next); }
		iterator end() const		{ return iterator(&_root); }

		inline bool empty() const	{ return _root.unlinked(); }
		size_t size() const			{ return std::distance(begin(), end()); }

		void push_back(IntrusiveListNode<T>& value)
		{ value.insert_before(&_root); }

		void erase(IntrusiveListNode<T>& value)
		{ value.unlink(); }
	};

	/** @} */


}



#endif
