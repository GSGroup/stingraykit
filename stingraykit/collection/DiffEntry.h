#ifndef STINGRAYKIT_COLLECTION_DIFFENTRY_H
#define STINGRAYKIT_COLLECTION_DIFFENTRY_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/compare/MemberListComparer.h>
#include <stingraykit/collection/CollectionOp.h>
#include <stingraykit/string/ToString.h>

namespace stingray
{

	template < typename T >
	struct DiffEntry
	{
		STINGRAYKIT_DEFAULTCOPYABLE(DiffEntry);
		STINGRAYKIT_DEFAULTMOVABLE(DiffEntry);

	public:
		using ItemType = T;

	public:
		CollectionOp	Op;
		ItemType		Item;

	public:
		template <
				typename ItemType__ = ItemType,
				typename EnableIf<IsConstructible<ItemType, const ItemType__&>::Value, int>::ValueT = 0
		>
		DiffEntry(CollectionOp op, const ItemType& item)
			: Op(op), Item(item)
		{ }

		template <
				typename ItemType__ = ItemType,
				typename EnableIf<IsConstructible<ItemType, ItemType__>::Value, int>::ValueT = 0
		>
		DiffEntry(CollectionOp op, ItemType&& item)
			: Op(op), Item(std::forward<ItemType__>(item))
		{ }

		template <
				typename ItemType__,
				typename EnableIf<IsConstructible<ItemType, const ItemType__&>::Value && IsConvertible<const ItemType__&, ItemType>::Value, int>::ValueT = 0
		>
		DiffEntry(const DiffEntry<ItemType__>& entry)
			: Op(entry.Op), Item(entry.Item)
		{ }

		template <
				typename ItemType__,
				typename EnableIf<IsConstructible<ItemType, const ItemType__&>::Value && !IsConvertible<const ItemType__&, ItemType>::Value, int>::ValueT = 0
		>
		explicit DiffEntry(const DiffEntry<ItemType__>& entry)
			: Op(entry.Op), Item(entry.Item)
		{ }

		template <
				typename ItemType__,
				typename EnableIf<IsConstructible<ItemType, ItemType__>::Value && IsConvertible<ItemType__, ItemType>::Value, int>::ValueT = 0
		>
		DiffEntry(DiffEntry<ItemType__>&& entry)
			: Op(std::move(entry.Op)), Item(std::forward<ItemType__>(entry.Item))
		{ }

		template <
				typename ItemType__,
				typename EnableIf<IsConstructible<ItemType, ItemType__>::Value && !IsConvertible<ItemType__, ItemType>::Value, int>::ValueT = 0
		>
		explicit DiffEntry(DiffEntry<ItemType__>&& entry)
			: Op(std::move(entry.Op)), Item(std::forward<ItemType__>(entry.Item))
		{ }

		int Compare(const DiffEntry& other) const
		{ return CompareMembersCmp(&DiffEntry::Op, &DiffEntry::Item)(*this, other); }

		std::string ToString() const
		{ return StringBuilder() % "{ " % Op % ": " % Item % " }"; }
	};


	template < typename T >
	DiffEntry<typename Decay<T>::ValueT> MakeDiffEntry(CollectionOp op, T&& item)
	{ return DiffEntry<typename Decay<T>::ValueT>(op, std::forward<T>(item)); }


}

#endif

