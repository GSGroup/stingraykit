#ifndef STINGRAYKIT_COLLECTION_ORDERED_SET_H
#define STINGRAYKIT_COLLECTION_ORDERED_SET_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/CollectionHelpers.h>

#include <list>
#include <set>

namespace stingray
{

	template < class Key, class Compare = std::less<Key>, class Allocator = std::allocator<Key> >
	class ordered_set
	{
		STINGRAYKIT_DEFAULTMOVABLE(ordered_set);

	public:
		using key_type = Key;
		using value_type = Key;

	private:
		struct ValueEntry;

		using ValueEntryAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<ValueEntry>;
		using ValueEntryHolder = AllocatorValueHolder<ValueEntry, ValueEntryAllocator>;

		struct CompareImpl
		{
			using is_transparent = void;

			Compare						Cmp;

			CompareImpl() { }
			CompareImpl(Compare comp) : Cmp(comp) { }

			bool operator () (ValueEntry* lhs, ValueEntry* rhs) const;
			bool operator () (ValueEntry* lhs, const Key& rhs) const;
			bool operator () (const Key& lhs, ValueEntry* rhs) const;
		};

		using OrderedAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<ValueEntryHolder>;
		using OrderedContainer = std::list<ValueEntryHolder, OrderedAllocator>;
		using OrderedIterator = typename OrderedContainer::iterator;
		using OrderedConstIterator = typename OrderedContainer::const_iterator;

		using SortedAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<ValueEntry*>;
		using SortedContainer = std::set<ValueEntry*, CompareImpl, SortedAllocator>;
		using SortedIterator = typename SortedContainer::iterator;
		using SortedConstIterator = typename SortedContainer::const_iterator;

		struct ValueEntry
		{
			OrderedIterator				OrderedIt;
			SortedIterator				SortedIt;
			value_type					Value;

			template < typename... Ts >
			ValueEntry(Ts&&... args)
				: Value(std::forward<Ts>(args)...)
			{ }
		};

		struct EqualCmp
		{
			bool operator () (const ValueEntryHolder& lhs, const ValueEntryHolder& rhs) const
			{ return lhs->Value == rhs->Value; }
		};

		struct LessCmp
		{
			bool operator () (const ValueEntryHolder& lhs, const ValueEntryHolder& rhs) const
			{ return lhs->Value < rhs->Value; }
		};

		class Iterator : public iterator_base<Iterator, const value_type, std::bidirectional_iterator_tag>
		{
			using base = iterator_base<Iterator, const value_type, std::bidirectional_iterator_tag>;

			friend class ordered_set;

		private:
			OrderedConstIterator		_implIt;

		public:
			Iterator()
			{ }

			Iterator(const OrderedIterator& implIt)
				: _implIt(implIt)
			{ }

			Iterator(const OrderedConstIterator& implIt)
				: _implIt(implIt)
			{ }

			typename base::reference dereference() const
			{ return (*_implIt)->Value; }

			bool equal(const Iterator& other) const
			{ return _implIt == other._implIt; }

			void increment()
			{ ++_implIt; }

			void decrement()
			{ --_implIt; }
		};

	public:
		using size_type = typename OrderedContainer::size_type;
		using difference_type = typename OrderedContainer::difference_type;
		using key_compare = Compare;
		using value_compare = Compare;
		using allocator_type = Allocator;
		using reference = typename Allocator::reference;
		using const_reference = typename Allocator::const_reference;
		using pointer = typename Allocator::pointer;
		using const_pointer = typename Allocator::const_pointer;
		using iterator = Iterator;
		using const_iterator = Iterator;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	private:
		ValueEntryAllocator			_alloc;
		OrderedContainer			_ordered;
		SortedContainer				_sorted;

	public:
		ordered_set()
		{ }

		explicit ordered_set(const Compare& comp, const Allocator& alloc = Allocator())
			: _alloc(alloc), _ordered(alloc), _sorted(comp, alloc)
		{ }

		explicit ordered_set(const Allocator& alloc)
			: ordered_set(Compare(), alloc)
		{ }

		template < class InputIterator >
		ordered_set(InputIterator first, InputIterator last, const Compare& comp = Compare(), const Allocator& alloc = Allocator())
			: _alloc(alloc), _ordered(alloc), _sorted(comp, alloc)
		{ insert(first, last); }

		template < class InputIterator >
		ordered_set(InputIterator first, InputIterator last, const Allocator& alloc)
			: ordered_set(first, last, Compare(), alloc)
		{ }

		ordered_set(const ordered_set& other)
			: ordered_set(other.begin(), other.end(), other._alloc)
		{ }

		ordered_set(const ordered_set& other, const Allocator& alloc)
			: ordered_set(other.begin(), other.end(), alloc)
		{ }

		ordered_set(ordered_set&& other, const Allocator& alloc)
			: _alloc(alloc), _ordered(std::move(other._ordered), alloc), _sorted(std::move(other._sorted), alloc)
		{ }

		ordered_set(std::initializer_list<value_type> list, const Compare& comp = Compare(), const Allocator& alloc = Allocator())
			: _alloc(alloc), _ordered(alloc), _sorted(comp, alloc)
		{ insert(list.begin(), list.end()); }

		ordered_set(std::initializer_list<value_type> list, const Allocator& alloc)
			: ordered_set(list, Compare(), alloc)
		{ }

		ordered_set& operator = (const ordered_set& other)
		{
			clear();
			insert(other.begin(), other.end());
			return *this;
		}

		ordered_set& operator = (std::initializer_list<value_type> list)
		{
			clear();
			insert(list.begin(), list.end());
			return *this;
		}

		allocator_type get_allocator() const	{ return _ordered.get_allocator(); }

		iterator begin()						{ return _ordered.begin(); }
		const_iterator begin() const			{ return _ordered.begin(); }
		const_iterator cbegin() const			{ return _ordered.cbegin(); }

		iterator end()							{ return _ordered.end(); }
		const_iterator end() const				{ return _ordered.end(); }
		const_iterator cend() const				{ return _ordered.cend(); }

		reverse_iterator rbegin()				{ return _ordered.rbegin(); }
		const_reverse_iterator rbegin() const	{ return _ordered.rbegin(); }
		const_reverse_iterator crbegin() const	{ return _ordered.crbegin(); }

		reverse_iterator rend()					{ return _ordered.rend(); }
		const_reverse_iterator rend() const		{ return _ordered.rend(); }
		const_reverse_iterator crend() const	{ return _ordered.crend(); }

		bool empty() const						{ return _ordered.empty(); }
		size_type size() const					{ return _ordered.size(); }
		size_type max_size() const				{ return std::min(_ordered.max_size(), _sorted.max_size()); }

		void clear()
		{
			_sorted.clear();
			_ordered.clear();
		}

		std::pair<iterator, bool> insert(const value_type& value)
		{ return DoInsertValue(_ordered.end(), value); }

		std::pair<iterator, bool> insert(value_type&& value)
		{ return DoInsertValue(_ordered.end(), std::move(value)); }

		iterator insert(const_iterator hint, const value_type& value)
		{ return DoInsertValue(hint._implIt, value).first; }

		iterator insert(const_iterator hint, value_type&& value)
		{ return DoInsertValue(hint._implIt, std::move(value)).first; }

		template < class InputIterator >
		void insert(InputIterator first, InputIterator last)
		{
			while (first != last)
				insert(*(first++));
		}

		void insert(std::initializer_list<value_type> list)
		{ insert(list.begin(), list.end()); }

		template < typename... Ts >
		std::pair<iterator, bool> emplace(Ts&&... args)
		{ return DoInsertEntry(_ordered.end(), ValueEntryHolder::create(_alloc, std::forward<Ts>(args)...)); }

		template < typename... Ts >
		iterator emplace_hint(const_iterator hint, Ts&&... args)
		{ return DoInsertEntry(hint._implIt, ValueEntryHolder::create(_alloc, std::forward<Ts>(args)...)).first; }

		iterator erase(const_iterator pos)
		{
			_sorted.erase((*pos._implIt)->SortedIt);
			return _ordered.erase(pos._implIt);
		}

		iterator erase(const_iterator first, const_iterator last)
		{
			while (first != last)
				first = erase(first);
			return first != end() ? (*first._implIt)->OrderedIt : end();
		}

		size_type erase(const Key& key)
		{
			const iterator pos = find(key);
			if (pos == end())
				return 0;

			erase(pos);
			return 1;
		}

		void swap(ordered_set& other)
		{
			_ordered.swap(other._ordered);
			_sorted.swap(other._sorted);
		}

		size_type count(const Key& key) const
		{ return find(key) == end() ? 0 : 1; }

		iterator find(const Key& key)
		{
			const SortedIterator result = _sorted.find(key);
			return result != _sorted.end() ? (*result)->OrderedIt : end();
		}

		const_iterator find(const Key& key) const
		{
			const SortedConstIterator result = _sorted.find(key);
			return result != _sorted.end() ? (*result)->OrderedIt : end();
		}

		key_compare key_comp() const			{ return _sorted.key_comp().Cmp; }
		value_compare value_comp() const		{ return _sorted.value_comp().Cmp; }

		template < class K, class C, class A >
		friend bool operator == (const ordered_set<K, C, A>& lhs, const ordered_set<K, C, A>& rhs);
		template < class K, class C, class A >
		friend bool operator < (const ordered_set<K, C, A>& lhs, const ordered_set<K, C, A>& rhs);

	private:
		template < typename Value_ >
		std::pair<iterator, bool> DoInsertValue(OrderedConstIterator orderedPos, Value_&& value)
		{
			const SortedConstIterator sortedPos = _sorted.lower_bound(value);

			if (sortedPos == _sorted.end() || _sorted.key_comp()(value, *sortedPos))
				return DoInsertAtPos(orderedPos, sortedPos, ValueEntryHolder::create(_alloc, std::forward<Value_>(value)));

			return std::make_pair((*sortedPos)->OrderedIt, false);
		}

		std::pair<iterator, bool> DoInsertEntry(OrderedConstIterator orderedPos, ValueEntryHolder&& valueEntry)
		{
			const SortedConstIterator sortedPos = _sorted.lower_bound(valueEntry.get());

			if (sortedPos == _sorted.end() || _sorted.key_comp()(valueEntry.get(), *sortedPos))
				return DoInsertAtPos(orderedPos, sortedPos, std::move(valueEntry));

			return std::make_pair((*sortedPos)->OrderedIt, false);
		}

		std::pair<iterator, bool> DoInsertAtPos(OrderedConstIterator orderedPos, SortedConstIterator sortedPos, ValueEntryHolder&& valueEntry_)
		{
			ValueEntry* const valueEntry = valueEntry_.get();
			valueEntry->OrderedIt = _ordered.insert(orderedPos, std::move(valueEntry_));

			try
			{ valueEntry->SortedIt = _sorted.insert(sortedPos, valueEntry); }
			catch (...)
			{
				_ordered.erase(valueEntry->OrderedIt);
				throw;
			}

			return std::make_pair(valueEntry->OrderedIt, true);
		}
	};


	template < class K, class C, class A >
	bool ordered_set<K, C, A>::CompareImpl::operator () (ValueEntry* lhs, ValueEntry* rhs) const
	{ return Cmp(lhs->Value, rhs->Value); }


	template < class K, class C, class A >
	bool ordered_set<K, C, A>::CompareImpl::operator () (ValueEntry* lhs, const K& rhs) const
	{ return Cmp(lhs->Value, rhs); }


	template < class K, class C, class A >
	bool ordered_set<K, C, A>::CompareImpl::operator () (const K& lhs, ValueEntry* rhs) const
	{ return Cmp(lhs, rhs->Value); }


	template < class K, class C, class A >
	bool operator == (const ordered_set<K, C, A>& lhs, const ordered_set<K, C, A>& rhs)
	{ return std::equal(lhs._ordered.begin(), lhs._ordered.end(), rhs._ordered.begin(), rhs._ordered.end(), typename ordered_set<K, C, A>::EqualCmp()); }
	STINGRAYKIT_GENERATE_NON_MEMBER_EQUALITY_OPERATORS_FROM_EQUAL(MK_PARAM(template < class K, class C, class A >), MK_PARAM(ordered_set<K, C, A>), MK_PARAM(ordered_set<K, C, A>));


	template < class K, class C, class A >
	bool operator < (const ordered_set<K, C, A>& lhs, const ordered_set<K, C, A>& rhs)
	{ return std::lexicographical_compare(lhs._ordered.begin(), lhs._ordered.end(), rhs._ordered.begin(), rhs._ordered.end(), typename ordered_set<K, C, A>::LessCmp()); }
	STINGRAYKIT_GENERATE_NON_MEMBER_RELATIONAL_OPERATORS_FROM_LESS(MK_PARAM(template < class K, class C, class A >), MK_PARAM(ordered_set<K, C, A>), MK_PARAM(ordered_set<K, C, A>));

}

#endif
