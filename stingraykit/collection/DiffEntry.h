#ifndef STINGRAYKIT_COLLECTION_DIFFENTRY_H
#define STINGRAYKIT_COLLECTION_DIFFENTRY_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/compare/MemberListComparer.h>
#include <stingraykit/string/ToString.h>

namespace stingray
{

	template< typename T >
	struct DiffEntry
	{
		typedef T ItemType;

		CollectionOp	Op;
		ItemType		Item;

		DiffEntry(CollectionOp op, const ItemType& item)
			: Op(op), Item(item)
		{ }

		int Compare(const DiffEntry& other) const
		{ return CompareMembersCmp(&DiffEntry::Op, &DiffEntry::Item)(*this, other); }

		std::string ToString() const
		{ return StringBuilder() % "DiffEntry { op: " % Op % ", item: " % Item % " }"; }
	};


	template< typename T >
	DiffEntry<T> MakeDiffEntry(CollectionOp op, const T& item)
	{ return DiffEntry<T>(op, item); }


}

#endif

