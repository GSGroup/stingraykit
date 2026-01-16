#ifndef STINGRAYKIT_COMPARE_CUSTOMCOMPARABLE_H
#define STINGRAYKIT_COMPARE_CUSTOMCOMPARABLE_H

#include <typeinfo>

#include <stddef.h>

namespace stingray
{

	template < typename Tag_ >
	struct ICustomComparable
	{
		virtual ~ICustomComparable() { }

		virtual int CustomCompare(const ICustomComparable<Tag_>& other) const = 0;
	};


	template < typename Tag_, typename T >
	struct CustomComparable : public virtual ICustomComparable<Tag_>
	{
	protected:
		int CustomCompare(const ICustomComparable<Tag_>& other) const override
		{
			if (this == &other)
				return 0;

			const std::type_info& thisType = typeid(*this);
			const std::type_info& otherType = typeid(other);

			if (thisType != otherType)
				return thisType.before(otherType) ? -1 : 1;

			//avoiding dynamic_cast here
			const char* thisPtr = reinterpret_cast<const char*>(this);
			const char* thisIComparable = reinterpret_cast<const char*>(static_cast<const ICustomComparable<Tag_>*>(this));
			const ptrdiff_t delta = thisPtr - thisIComparable; //distance between CustomComparable and ICustomComparable for this type.
			const char* otherIComparable = reinterpret_cast<const char*>(&other);
			const CustomComparable<Tag_, T>* otherPtr = reinterpret_cast<const CustomComparable<Tag_, T>*>(otherIComparable + delta);

			return DoCustomCompare(Tag_(), *static_cast<const T*>(otherPtr));
		}

		virtual int DoCustomCompare(Tag_, const T& other) const = 0;
	};


	template < typename Tag_, typename T >
	struct IndifferentlyCustomComparable : public CustomComparable<Tag_, T>
	{
	protected:
		int DoCustomCompare(Tag_, const T& other) const override
		{ return 0; }
	};

}

#endif
