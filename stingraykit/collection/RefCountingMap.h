#ifndef STINGRAYKIT_COLLECTION_REFCOUNTINGMAP_H
#define STINGRAYKIT_COLLECTION_REFCOUNTINGMAP_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/iterator_base.h>
#include <stingraykit/compare/comparers.h>
#include <stingraykit/optional.h>

#include <map>

namespace stingray
{

	template < typename Key_, typename Value_, typename Comparer_ = comparers::Less >
	class RefCountingMap
	{
		static_assert(comparers::IsRelationalComparer<Comparer_>::Value, "Expected Relational comparer");

		struct ValueHolder
		{
			size_t	References;
			Value_	Value;

			template < typename DoAddFunc >
			ValueHolder(const Key_& key, const DoAddFunc& doAddFunc)
				:	References(1),
					Value(doAddFunc(key))
			{ }
		};

		using Impl = std::map<Key_, ValueHolder, Comparer_>;

		template < bool Const >
		struct Iterator : public iterator_base<Iterator<Const>, std::pair<const Key_&, typename If<Const, const Value_&, Value_&>::ValueT>, std::bidirectional_iterator_tag>
		{
			using PairType = std::pair<const Key_&, typename If<Const, const Value_&, Value_&>::ValueT>;
			using base = iterator_base<Iterator, PairType, std::bidirectional_iterator_tag>;

			using ImplIterator = typename If<Const, typename Impl::const_iterator, typename Impl::iterator>::ValueT;

		private:
			ImplIterator				_implIt;
			mutable optional<PairType>	_pair;

		public:
			Iterator() { }

			template < typename ImplIt >
			Iterator(const ImplIt& implIt) : _implIt(implIt) { }

			template < bool Const_ >
			Iterator(const Iterator<Const_>& it) : _implIt(it.GetImpl()) { }

			Iterator& operator = (const Iterator& other)
			{ _implIt = other._implIt; _pair.reset(); return *this; }

			typename base::reference dereference() const
			{
				if (!_pair)
					_pair.emplace(_implIt->first, _implIt->second.Value);
				return *_pair;
			}

			template < bool Const_>
			bool equal(const Iterator<Const_>& other) const
			{ return _implIt == other._implIt; }

			void increment()
			{
				++_implIt;
				_pair.reset();
			}

			ImplIterator GetImpl() const
			{ return _implIt; }
		};

	public:
		using iterator = Iterator<false>;
		using const_iterator = Iterator<true>;

	private:
		Impl		_impl;

	public:
		bool empty() const							{ return _impl.empty(); }
		size_t size() const							{ return _impl.size(); }

		iterator begin()							{ return _impl.begin(); }
		iterator end()								{ return _impl.end(); }

		const_iterator begin() const				{ return _impl.begin(); }
		const_iterator end() const 					{ return _impl.end(); }

		iterator find(const Key_& key)				{ return _impl.find(key); }
		const_iterator find(const Key_& key) const	{ return _impl.find(key); }

		size_t count(const Key_& key) const
		{
			const typename Impl::const_iterator it = _impl.find(key);
			return it != _impl.end() ? it->second.References : 0;
		}

		template < typename DoAddFunc >
		iterator add(const Key_& key, const DoAddFunc& doAddFunc)
		{
			const typename Impl::iterator it = _impl.find(key);
			if (it != _impl.end())
			{
				++it->second.References;
				return it;
			}

			return _impl.emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(key, doAddFunc)).first;
		}

		template < typename DoRemoveFunc >
		size_t erase(const Key_& key, const DoRemoveFunc& doRemoveFunc)
		{
			const typename Impl::iterator implIt = _impl.find(key);
			if (implIt == _impl.end())
				return 0;

			if (--implIt->second.References > 0)
				return 1;

			doRemoveFunc(implIt->first, implIt->second.Value);
			_impl.erase(implIt);
			return 1;
		}

		template < typename DoRemoveFunc >
		void erase(iterator it, const DoRemoveFunc& doRemoveFunc)
		{
			const typename Impl::iterator implIt = it.GetImpl();
			if (--implIt->second.References > 0)
				return;

			doRemoveFunc(implIt->first, implIt->second.Value);
			_impl.erase(implIt);
		}

		template < typename DoRemoveFunc >
		void clear(const DoRemoveFunc& doRemoveFunc)
		{
			for (typename Impl::iterator it = _impl.begin(); it != _impl.end(); ++it)
				doRemoveFunc(it->first, it->second.Value);
			_impl.clear();
		}
	};

}

#endif
