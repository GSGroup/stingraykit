#ifndef STINGRAYKIT_COLLECTION_COLLECTIONBUILDER_H
#define STINGRAYKIT_COLLECTION_COLLECTIONBUILDER_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/metaprogramming/TypeTraits.h>

#include <map>
#include <set>
#include <vector>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename CollectionType >
	class CollectionBuilder
	{
	public:
		typedef typename AddConstLvalueReference<typename CollectionType::value_type>::ValueT ArgType;

	private:
		CollectionType		_collection;

	public:
		template < typename... Ts >
		CollectionBuilder(const Ts&... args) { Insert(args...); }

		const CollectionType& Get() const { return _collection; }

		operator const CollectionType& () const { return Get(); }

		CollectionBuilder& operator % (ArgType val)
		{ InsertImpl(_collection, val, 0); return *this; }

	private:
		template < typename... Ts >
		void Insert(const ArgType& a0, const Ts&... args) { *this % a0; Insert(args...); }
		void Insert() { }

		template < typename CollectionType_ >
		static auto InsertImpl(CollectionType_& collection, ArgType val, int)
				-> decltype(collection.push_back(val), void())
		{ collection.push_back(val); }

		template < typename CollectionType_ >
		static void InsertImpl(CollectionType_& collection, ArgType val, long)
		{ collection.insert(val); }
	};


	template < typename T >
	using VectorBuilder = CollectionBuilder<std::vector<T>>;

	template < typename T >
	using SetBuilder = CollectionBuilder<std::set<T>>;

	template < typename K, typename V >
	using MapBuilder = CollectionBuilder<std::map<K, V>>;

	/** @} */

}

#endif
