#ifndef __GS_DVRLIB_TOOLKIT_INTRUSIVE_LIST_H__
#define __GS_DVRLIB_TOOLKIT_INTRUSIVE_LIST_H__

#include <stingray/toolkit/toolkit.h>
#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/unique_ptr.h>
#include <stingray/toolkit/iterator_base.h>
/*! \cond GS_INTERNAL */

namespace stingray
{


	template < typename T, typename Alloc = std::allocator<T> >
	class intrusive_list;


	template < typename T >
	class intrusive_list_node
	{
		template <typename U, typename A> friend class intrusive_list;

		intrusive_list_node		*_prev, *_next;

	protected:
		inline void unlink()
		{
			_next->_prev = _prev;
			_prev->_next = _next;
			_prev = _next = this;
		}

		inline bool unlinked() const { return _next == this; }

		inline void insert_before(intrusive_list_node *node)
		{
			_prev = node->_prev;
			_next = node;

			_next->_prev = _prev->_next = this;
		}

		inline void insert_after(intrusive_list_node *node)
		{
			_prev = node;
			_next = node->_next;

			_next->_prev = _prev->_next = this;
		}

		void swap(intrusive_list_node& other)
		{
			std::swap(_prev, other._prev);
			std::swap(_next, other._next);
		}

		inline intrusive_list_node()
			: _prev(this), _next(this)
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
		typedef intrusive_list_node<T>	node_type;

		typedef T						value_type;
		typedef node_type*				pointer_type;
		typedef const node_type*		const_pointer_type;
		typedef node_type&				reference_type;
		typedef const node_type&		const_reference_type;

		typedef Allocator_				allocator_type;

	public:
		class iterator : public iterator_base<iterator, value_type, std::bidirectional_iterator_tag> // TODO: Reimplement this iterator using iterator_base
		{
			typedef iterator_base<iterator, value_type, std::bidirectional_iterator_tag>	base;

		public:
			typedef typename base::pointer													pointer;
			typedef typename base::reference												reference;
			typedef typename base::difference_type											difference_type;

		private:
			pointer_type					_current;

		public:
			iterator(pointer_type current)
				: _current(current)
			{ }

			reference dereference() const										{ return static_cast<reference>(*_current); }
			bool equal(const iterator &other) const								{ return _current == other._current; }
			void increment()													{ _current = get_next(_current); }
			void decrement()													{ _current = get_prev(_current); }

			pointer_type get()													{ return _current; }
			const_pointer_type get() const										{ return _current; }
		};
		//fixme: add const_iterators here!
		typedef iterator const_iterator;

	private:
		mutable intrusive_list_node<T>	_root;
		allocator_type					_alloc;

		static node_type* get_next(const node_type* n) { return n->_next; }
		static node_type* get_prev(const node_type* n) { return n->_prev; }

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

		inline void fix_root(const intrusive_list &other)
		{
			if (_root._next == &other._root)
				_root._prev = _root._next = &_root;
			else
				_root._next->_prev = _root._prev->_next = &_root;
		}

	public:
		explicit intrusive_list(const allocator_type &alloc = allocator_type())
			: _root(), _alloc(alloc)
		{ }

		intrusive_list(const intrusive_list& other)
			: _root(), _alloc(other._alloc)
		{
			for (const_iterator it = other.begin(); it != other.end(); ++it)
				push_back(*it);
		}

		~intrusive_list()
		{
			pointer_type p(_root._next);
			while (p != &_root)
			{
				pointer_type tmp(p);
				p = tmp->_next;
				destroy_node(tmp);
			}
		}

		intrusive_list& operator = (const intrusive_list& other)
		{
			if (this == &other)
				return *this;

			intrusive_list tmp(other);
			swap(tmp);

			return *this;
		}


		void swap(intrusive_list& other) 
		{
			_root.swap(other._root);
			fix_root(other);
			other.fix_root(*this);
			std::swap(_alloc, other._alloc);
		}

		iterator begin()		{ return iterator(_root._next); }
		iterator end()			{ return iterator(&_root); }

		// Sorry. =)
		iterator begin() const	{ return iterator(_root._next); }
		iterator end() const	{ return iterator(&_root); }

		inline bool empty() const	{ return _root.unlinked(); }
	
		void push_back(const T& value)
		{
			pointer_type p = create_node(value);
			p->insert_before(&_root);
		}

		void erase(const iterator& it)
		{
			if (it == end())
				TOOLKIT_THROW("destroying iterator pointing to the end");

			pointer_type p = const_cast<pointer_type>(it.get());
			p->unlink();
			destroy_node(p);
		}

		size_t size() const
		{
			size_t n = 0;
			for(const_iterator i = begin(); i != end(); ++i)
				++n;
			return n;
		}
	};


}

/*! \endcond */


#endif
