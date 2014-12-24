#ifndef STINGRAY_TOOLKIT_IVISITOR_H
#define STINGRAY_TOOLKIT_IVISITOR_H


#include <stingray/toolkit/shared_ptr.h>


namespace stingray
{

	template < typename BaseType >
	struct IVisitor
	{
		virtual ~IVisitor() { }
	};

	template < typename BaseType, typename DerivedType >
	struct Visitor : public virtual IVisitor<BaseType>
	{
		virtual void Visit(DerivedType& visitable) = 0;
	};

	template < typename BaseType >
	struct IVisitable
	{
		virtual ~IVisitable() { }

		virtual void Accept(IVisitor<BaseType>& visitor) = 0;
		virtual void Accept(IVisitor<const BaseType>& visitor) const = 0;
	};

	template < typename BaseType, typename DerivedType >
	struct Visitable : public virtual IVisitable<BaseType>
	{
		virtual void Accept(IVisitor<BaseType>& visitor)
		{ dynamic_cast<Visitor<BaseType, DerivedType>&>(visitor).Visit(*static_cast<DerivedType*>(this)); }

		virtual void Accept(IVisitor<const BaseType>& visitor) const
		{ dynamic_cast<Visitor<const BaseType, const DerivedType>&>(visitor).Visit(*static_cast<const DerivedType*>(this)); }
	};

}


#endif
