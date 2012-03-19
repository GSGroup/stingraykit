#ifndef __GS_DVRLIB_TOOLKIT_INTRUSIVE_LIST_H__
#define __GS_DVRLIB_TOOLKIT_INTRUSIVE_LIST_H__

#include <dvrlib/toolkit/toolkit.h>
#include <dvrlib/toolkit/exception.h>
/*! \cond GS_INTERNAL */

namespace dvrlib
{


	template < typename T, typename Alloc = std::allocator<T> >
	class intrusive_list;


	template < typename T >
	class intrusive_list_node
	{
		template <typename U, typename A> friend class intrusive_list;
	public:
		intrusive_list_node*	_next;
		intrusive_list_node*	_prev;

	protected:
		intrusive_list_node() 
			: _next(NULL), _prev(NULL)
		{ }

		~intrusive_list_node() { }
	};


	template < typename T >
	class intrusive_list_node_wrapper : public intrusive_list_node<intrusive_list_node_wrapper<T> >
	{
	private:
		T	_data;

	public:
		intrusive_list_node_wrapper(const T& data)
			: _data(data)
		{ }

		operator T& ()				{ return _data; }
		operator const T& () const	{ return _data; }
	};


	template < typename T , typename Allocator_ >
	class intrusive_list
	{
	public:
		typedef T			value_type;
		typedef Allocator_	allocator_type;

	public:
		class iterator //: iterator_base<iterator, value_type, std::bidirectional_iterator_tag> TODO: Reimplement this iterator using iterator_base
		{
		public:
			typedef T	value_type;

		private:
			intrusive_list_node<T>*		_current;

		public:
			iterator(intrusive_list_node<T>* current)
				: _current(current)
			{ }

			bool operator == (const iterator& other) const { return _current == other._current; }
			bool operator != (const iterator& other) const { return !(*this == other); }

			T& operator * () const	{ return static_cast<T&>(*_current); }
			T* operator -> () const	{ return static_cast<T*>(_current); }

			const iterator& operator ++() { _current = _current->_next;  return *this; }
			const iterator& operator --() { _current = _current->_prev;  return *this; }
		};
		//fixme: add const_iterators here!
		typedef iterator const_iterator;

	private:
		mutable intrusive_list_node<T>	_tail;
		intrusive_list_node<T>*		_head;
		size_t						_size;
		allocator_type				_alloc;

		T * create_node(const T& value)
		{
			T * node = _alloc.allocate(1);
			_alloc.construct(node, value);
			return node;
		}

		void destroy_node(intrusive_list_node<T> *node)
		{
			T *t = static_cast<T *>(node);
			_alloc.destroy(t);
			_alloc.deallocate(t, 1);
		}

	public:
		explicit intrusive_list(const allocator_type &alloc = allocator_type())
			: _tail(), _head(&_tail), _size(0), _alloc(alloc)
		{ }

		intrusive_list(const intrusive_list& other)
			: _tail(), _head(&_tail), _size(0), _alloc(other._alloc)
		{
			for (const_iterator it = other.begin(); it != other.end(); ++it)
				push_back(*it);
		}

		~intrusive_list()
		{
			intrusive_list_node<T>* n = _head;
			while (n != &_tail)
			{
				intrusive_list_node<T>* tmp = n;
				n = tmp->_next;
				destroy_node(tmp);
			}
		}


		intrusive_list& operator = (const intrusive_list& other)
		{
			intrusive_list tmp(other);
			swap(tmp);

			return *this;
		}


		void swap(intrusive_list& other) 
		{
			std::swap(_head, other._head);

			if (_tail._prev)
				_tail._prev->_next = &other._tail;
			else
				other._head = &other._tail;

			if (other._tail._prev)
				other._tail._prev->_next = &_tail;
			else
				_head = &_tail;

			std::swap(_tail._prev, other._tail._prev);
			std::swap(_size, other._size);
		}

		iterator begin()		{ return iterator(_head); }
		iterator end()			{ return iterator(&_tail); }

		// Sorry. =)
		iterator begin() const	{ return iterator(_head); }
		iterator end() const	{ return iterator(&_tail); }

		bool empty() const		{ return _head == &_tail; }
		size_t size() const		{ return _size; }
	
		void push_back(const T& value)
		{
			if (empty())
			{
				_head = create_node(value);
				_head->_next = &_tail;
				_tail._prev = _head;
			}
			else
			{
				intrusive_list_node<T>* tmp = _tail._prev;
				tmp->_next = _tail._prev = create_node(value);
				tmp->_next->_prev = tmp;
				_tail._prev->_next = &_tail;
			}
			++_size;
		}


		void erase(const iterator& it)
		{
			if (it == end())
				TOOLKIT_THROW(std::runtime_error("fail. =)"));

			intrusive_list_node<T>* n = it->_next;
			intrusive_list_node<T>* p = it->_prev;

			if (p == NULL) // it ~ _head
			{
				destroy_node(_head);
				n->_prev = NULL;
				_head = n;
			}
			else
			{
				destroy_node(p->_next);
				p->_next = n;
				n->_prev = p;
			}
			
			--_size;
		}
	};


}

/*! \endcond */


#endif
