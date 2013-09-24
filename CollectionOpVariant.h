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


		struct Op
		{
			TOOLKIT_ENUM_VALUES(Added = 1, Removed, Modified); // Should be the same as indices in the typelist
			TOOLKIT_DECLARE_ENUM_CLASS(Op);
		};

	private:
		typedef TYPELIST(EmptyType, Added, Removed, Modified)	Types;

		variant<Types>		_data;

	public:
		CollectionOpVariant()
			: _data()
		{ }

		template < typename ParamType >
		CollectionOpVariant(const ParamType& p)
			: _data(p)
		{ }

		Op GetOp() const
		{
			TOOLKIT_CHECK(_data.which() > 0, "Cannot access empty CollectionOpVariant!");
			return Op(_data.which());
		}

		template < typename T >
		const T& As() const
		{ return variant_get<T>(_data); }
	};


}

#endif
