#ifndef STINGRAYKIT_COLLECTION_REFCOUNTINGSET_H
#define STINGRAYKIT_COLLECTION_REFCOUNTINGSET_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/iterators.h>
#include <stingraykit/compare/comparers.h>

#include <map>

namespace stingray
{

	template < typename Key_, typename Comparer_ = comparers::Less >
	class RefCountingSet
	{
		static_assert(comparers::IsRelationalComparer<Comparer_>::Value, "Expected Relational comparer");

		using Impl = std::map<Key_, size_t, Comparer_>;

	public:
		using const_iterator = MapKeysIterator<typename Impl::const_iterator>;
		using iterator = MapKeysIterator<typename Impl::iterator>;

	private:
		Impl		_impl;

	public:
		bool empty() const							{ return _impl.empty(); }
		size_t size() const							{ return _impl.size(); }

		iterator begin()							{ return keys_iterator(_impl.begin()); }
		iterator end()								{ return keys_iterator(_impl.end()); }

		const_iterator begin() const				{ return keys_iterator(_impl.begin()); }
		const_iterator end() const 					{ return keys_iterator(_impl.end()); }

		iterator find(const Key_& key)				{ return keys_iterator(_impl.find(key)); }
		const_iterator find(const Key_& key) const	{ return keys_iterator(_impl.find(key)); }

		size_t count(const Key_& key) const
		{
			const typename Impl::const_iterator it = _impl.find(key);
			return it != _impl.end() ? it->second : 0;
		}

		template < typename DoAddFunc >
		iterator add(const Key_& key, const DoAddFunc& doAddFunc)
		{
			const typename Impl::iterator it = _impl.find(key);
			if (it != _impl.end())
			{
				++it->second;
				return keys_iterator(it);
			}

			doAddFunc(key);
			return keys_iterator(_impl.insert(std::make_pair(key, 1)).first);
		}

		template < typename DoRemoveFunc >
		size_t erase(const Key_& key, const DoRemoveFunc& doRemoveFunc)
		{
			const typename Impl::iterator it = _impl.find(key);
			if (it == _impl.end())
				return 0;

			if (--it->second == 0)
			{
				doRemoveFunc(it->first);
				_impl.erase(it);
			}
			return 1;
		}

		template < typename DoRemoveFunc >
		void erase(iterator it, const DoRemoveFunc& doRemoveFunc)
		{
			const typename Impl::iterator implIt = it.base();
			if (--implIt->second > 0)
				return;

			doRemoveFunc(implIt->first);
			_impl.erase(implIt);
		}

		template < typename DoRemoveFunc >
		void clear(const DoRemoveFunc& doRemoveFunc)
		{
			for (typename Impl::iterator it = _impl.begin(); it != _impl.end(); ++it)
				doRemoveFunc(it->first);
			_impl.clear();
		}
	};

}

#endif
