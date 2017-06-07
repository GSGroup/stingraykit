#ifndef STINGRAYKIT_COLLECTION_ENUMERABLEBUILDER_H
#define STINGRAYKIT_COLLECTION_ENUMERABLEBUILDER_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumeratorFromStlContainer.h>
#include <stingraykit/collection/IEnumerable.h>
#include <stingraykit/shared_ptr.h>

#include <vector>

namespace stingray
{

	template<typename ItemType>
	class EnumerableBuilder
	{
		typedef std::vector<ItemType> Container;

	private:
		shared_ptr<Container> _container;

	public:
		EnumerableBuilder() : _container(make_shared<Container>())
		{ }

		template <typename Iter>
		EnumerableBuilder(Iter first, Iter last) : _container(make_shared<Container>(first, last))
		{ }

		shared_ptr<IEnumerable<ItemType> > Get() const
		{ return EnumerableFromStlContainer(*_container, _container); }

		void Add(const ItemType& val)
		{ _container->push_back(val); }

		template <typename Iter>
		void Add(Iter first, Iter last)
		{ std::copy(first, last, std::back_inserter(_container)); }

		EnumerableBuilder& operator % (const ItemType& val)
		{ Add(val); return *this; }
	};

}

#endif
