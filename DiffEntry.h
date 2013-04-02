#ifndef STINGRAY_TOOLKIT_DIFFENTRY_H
#define STINGRAY_TOOLKIT_DIFFENTRY_H


#include <stingray/toolkit/toolkit.h>


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
	};


	template< typename T >
	DiffEntry<T> MakeDiffEntry(const T& t, CollectionOp op)
	{ return DiffEntry<T>(t, op); }


}


#endif

