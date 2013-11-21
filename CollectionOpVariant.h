#ifndef STINGRAY_TOOLKIT_COLLECTIONOPVARIANT_H
#define STINGRAY_TOOLKIT_COLLECTIONOPVARIANT_H


#include <stingray/toolkit/toolkit.h>
#include <stingray/toolkit/variant.h>


namespace stingray
{

	template < typename ItemType >
	class CollectionOpVariant
	{
	public:
		struct Added
		{
			ItemType Value;
			Added(const ItemType& value) : Value(value) { }
		};

		struct Removed
		{
			ItemType Value;
			Removed(const ItemType& value) : Value(value) { }
		};

		struct Modified
		{
			ItemType OldValue;
			ItemType NewValue;
			Modified(const ItemType& oldValue, const ItemType& newValue) : OldValue(oldValue), NewValue(newValue) { }
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
			default:											TOOLKIT_THROW("Cannot access empty CollectionOpVariant!");
			}
		}

		template < typename T >
		const T& As() const
		{ return variant_get<T>(_data); }

		const variant<Types>& GetVariant() const
		{ return _data; }
	};


}

#endif
