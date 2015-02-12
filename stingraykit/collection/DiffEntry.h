#ifndef STINGRAYKIT_COLLECTION_DIFFENTRY_H
#define STINGRAYKIT_COLLECTION_DIFFENTRY_H


#include <stingraykit/toolkit.h>


namespace stingray
{


	template< typename T >
	struct DiffEntry
	{
		typedef T ItemType;

		ItemType		Item;
		CollectionOp	Op;

		DiffEntry(const T &t, CollectionOp op) : Item(t), Op(op)
		{}

		bool operator == (const DiffEntry& other) const { return Item == other.Item && Op == other.Op; }
		bool operator != (const DiffEntry& other) const { return !(*this == other); }

		std::string ToString() const { return StringBuilder() % "DiffEntry { op: " % Op % ", item: " % Item % " }"; }
	};


	template< typename T >
	DiffEntry<T> MakeDiffEntry(const T& t, CollectionOp op)
	{ return DiffEntry<T>(t, op); }


}


#endif

