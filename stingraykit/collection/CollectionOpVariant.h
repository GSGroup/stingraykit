#ifndef STINGRAYKIT_COLLECTION_COLLECTIONOPVARIANT_H
#define STINGRAYKIT_COLLECTION_COLLECTIONOPVARIANT_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/toolkit.h>
#include <stingraykit/variant.h>


namespace stingray
{

	template < typename ItemType >
	class CollectionOpVariant
	{
	public:
		struct Added
		{
			ItemType Value;

			Added() { } // For deserialization only!
			Added(const ItemType& value) : Value(value) { }

			template <typename ArchiveType_> void Serialize(ArchiveType_& ar) const	{ ar.Serialize("value", Value); }
			template <typename ArchiveType_> void Deserialize(ArchiveType_& ar)		{ ar.Deserialize("value", Value); }
		};

		struct Removed
		{
			ItemType Value;

			Removed() { } // For deserialization only!
			Removed(const ItemType& value) : Value(value) { }

			template <typename ArchiveType_> void Serialize(ArchiveType_& ar) const	{ ar.Serialize("value", Value); }
			template <typename ArchiveType_> void Deserialize(ArchiveType_& ar)		{ ar.Deserialize("value", Value); }
		};

		struct Modified
		{
			ItemType OldValue;
			ItemType NewValue;

			Modified() { } // For deserialization only!
			Modified(const ItemType& oldValue, const ItemType& newValue) : OldValue(oldValue), NewValue(newValue) { }

			template <typename ArchiveType_> void Serialize(ArchiveType_& ar) const	{ ar.Serialize("oldValue", OldValue).Serialize("newValue", NewValue); }
			template <typename ArchiveType_> void Deserialize(ArchiveType_& ar)		{ ar.Deserialize("oldValue", OldValue).Deserialize("newValue", NewValue); }
		};

		typedef TypeList<EmptyType, Added, Removed, Modified>	Types;
		typedef variant<Types>									Variant;

	private:
		Variant		_data;

	public:
		CollectionOpVariant()
			: _data()
		{ }

		template < typename ParamType >
		CollectionOpVariant(const ParamType& p)
			: _data(p)
		{ }

		CollectionOp GetOp() const
		{
			switch (_data.which())
			{
			case IndexOfTypeListItem<Types, Added>::Value:		return CollectionOp::Added;
			case IndexOfTypeListItem<Types, Modified>::Value:	return CollectionOp::Updated;
			case IndexOfTypeListItem<Types, Removed>::Value:	return CollectionOp::Removed;
			default:											STINGRAYKIT_THROW("Cannot access empty CollectionOpVariant!");
			}
		}

		template < typename T >
		const T& As() const
		{ return variant_get<T>(_data); }

		const variant<Types>& GetVariant() const
		{ return _data; }

		static CollectionOpVariant CreateAdded(const ItemType& value)									{ return Added(value); }
		static CollectionOpVariant CreateRemoved(const ItemType& value)									{ return Removed(value); }
		static CollectionOpVariant CreateModified(const ItemType& oldValue, const ItemType& newValue)	{ return Modified(oldValue, newValue); }

		template <typename ArchiveType_> void Serialize(ArchiveType_& ar) const		{ ar.Serialize("data", _data); }
		template <typename ArchiveType_> void Deserialize(ArchiveType_& ar)			{ ar.Deserialize("data", _data); }
	};


}

#endif
