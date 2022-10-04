#ifndef STINGRAYKIT_COLLECTION_FLAT_SET_H
#define STINGRAYKIT_COLLECTION_FLAT_SET_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/toolkit.h>

#include <algorithm>

namespace stingray
{

	template < class Key, class Compare = std::less<Key>, class Allocator = std::allocator<Key> >
	class flat_set
	{
		STINGRAYKIT_DEFAULTCOPYABLE(flat_set);
		STINGRAYKIT_DEFAULTMOVABLE(flat_set);

	public:
		using key_type = Key;
		using value_type = Key;

	private:
		using Container = std::vector<value_type, Allocator>;

	public:
		using size_type = typename Container::size_type;
		using difference_type = typename Container::difference_type;
		using key_compare = Compare;
		using value_compare = Compare;
		using allocator_type = Allocator;
		using reference = typename Allocator::reference;
		using const_reference = typename Allocator::const_reference;
		using pointer = typename Allocator::pointer;
		using const_pointer = typename Allocator::const_pointer;
		using iterator = typename Container::const_iterator; // const_iterator isn't error here
		using const_iterator = typename Container::const_iterator;
		using reverse_iterator = typename Container::const_reverse_iterator;
		using const_reverse_iterator = typename Container::const_reverse_iterator;

	private:
		Container		_container;
		Compare			_cmp;

	public:
		explicit flat_set(const Compare& comp = Compare(), const Allocator& alloc = Allocator())
			: _container(alloc), _cmp(comp)
		{ }

		template < class InputIterator >
		flat_set(InputIterator first, InputIterator last, const Compare& comp = Compare(), const Allocator& alloc = Allocator())
			: _container(alloc), _cmp(comp)
		{ insert(first, last); }

		flat_set(std::initializer_list<value_type> list, const Compare& comp = Compare(), const Allocator& alloc = Allocator())
			: _container(alloc), _cmp(comp)
		{ insert(list.begin(), list.end()); }

		allocator_type get_allocator() const	{ return _container.get_allocator(); }

		iterator begin()						{ return _container.begin(); }
		const_iterator begin() const			{ return _container.begin(); }

		iterator end()							{ return _container.end(); }
		const_iterator end() const				{ return _container.end(); }

		reverse_iterator rbegin()				{ return _container.rbegin(); }
		const_reverse_iterator rbegin() const	{ return _container.rbegin(); }

		reverse_iterator rend()					{ return _container.rend(); }
		const_reverse_iterator rend() const		{ return _container.rend(); }

		bool empty() const						{ return _container.empty(); }
		size_type size() const					{ return _container.size(); }
		size_type max_size() const				{ return _container.max_size(); }

		void reserve(size_type size)			{ _container.reserve(size); }

		void clear()							{ _container.clear(); }

		std::pair<iterator, bool> insert(const value_type& value)
		{
			const iterator result(lower_bound(value));
			if (result == end() || _cmp(value, *result))
				return std::make_pair(_container.insert(result, value), true);
			return std::make_pair(result, false);
		}

		iterator insert(iterator hint, const value_type& value)
		{
			if ((hint == begin() || _cmp(*(hint - 1), value)) && (hint == end() || _cmp(value, *hint)))
				return _container.insert(hint, value);
			return insert(value).first;
		}

		template < class InputIterator >
		void insert(InputIterator first, InputIterator last)
		{
			while (first != last)
				insert(*(first++));
		}

		void erase(iterator pos)
		{ _container.erase(pos); }

		void erase(iterator first, iterator last)
		{ _container.erase(first, last); }

		size_type erase(const Key& key)
		{
			const iterator result(find(key));
			if (result == end())
				return 0;
			erase(result);
			return 1;
		}

		void swap(flat_set& other)
		{
			_container.swap(other._container);
			std::swap(_cmp, other._cmp);
		}

		size_type count(const Key& key) const
		{ return find(key) == end() ? 0 : 1; }

		iterator find(const Key& key)
		{
			const iterator result(lower_bound(key));
			if (result != end() && _cmp(key, *result))
				return end();
			return result;
		}

		const_iterator find(const Key& key) const
		{
			const const_iterator result(lower_bound(key));
			if (result != end() && _cmp(key, *result))
				return end();
			return result;
		}

		std::pair<iterator, iterator> equal_range(const Key& key)						{ return std::equal_range(begin(), end(), key, _cmp); }
		std::pair<const_iterator, const_iterator> equal_range(const Key& key) const		{ return std::equal_range(begin(), end(), key, _cmp); }

		iterator lower_bound(const Key& key)											{ return std::lower_bound(begin(), end(), key, _cmp); }
		const_iterator lower_bound(const Key& key) const								{ return std::lower_bound(begin(), end(), key, _cmp); }

		iterator upper_bound(const Key& key)											{ return std::upper_bound(begin(), end(), key, _cmp); }
		const_iterator upper_bound(const Key& key) const								{ return std::upper_bound(begin(), end(), key, _cmp); }

		key_compare key_comp() const													{ return _cmp; }
		value_compare value_comp() const												{ return _cmp; }

		template < class K, class C, class A > friend bool operator == (const flat_set<K, C, A>& lhs, const flat_set<K, C, A>& rhs);
		template < class K, class C, class A > friend bool operator < (const flat_set<K, C, A>& lhs, const flat_set<K, C, A>& rhs);
	};


	template < class K, class C, class A >
	bool operator == (const flat_set<K, C, A>& lhs, const flat_set<K, C, A>& rhs)
	{ return lhs._container == rhs._container; }
	STINGRAYKIT_GENERATE_NON_MEMBER_EQUALITY_OPERATORS_FROM_EQUAL(MK_PARAM(template < class K, class C, class A >), MK_PARAM(flat_set<K, C, A>), MK_PARAM(flat_set<K, C, A>));


	template < class K, class C, class A >
	bool operator < (const flat_set<K, C, A>& lhs, const flat_set<K, C, A>& rhs)
	{ return lhs._container < rhs._container; }
	STINGRAYKIT_GENERATE_NON_MEMBER_RELATIONAL_OPERATORS_FROM_LESS(MK_PARAM(template < class K, class C, class A >), MK_PARAM(flat_set<K, C, A>), MK_PARAM(flat_set<K, C, A>));

}

#endif

