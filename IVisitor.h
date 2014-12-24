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

	template < typename BaseType >
	struct IVisitorByPtr
	{
		virtual ~IVisitorByPtr() { }
	};

	template < typename BaseType, typename DerivedType >
	struct VisitorByPtr : public virtual IVisitorByPtr<BaseType>
	{
		virtual void Visit(const shared_ptr<DerivedType>& visitable) = 0;
	};

	template < typename BaseType >
	struct IVisitableByPtr
	{
		virtual ~IVisitableByPtr() { }

		virtual void AcceptPtr(IVisitorByPtr<BaseType>& visitor, const shared_ptr<BaseType>& thisptr) = 0;
		virtual void AcceptPtr(IVisitorByPtr<const BaseType>& visitor, const shared_ptr<const BaseType>& thisptr) const = 0;
	};

	template < typename BaseType, typename DerivedType >
	struct VisitableByPtr : public virtual IVisitableByPtr<BaseType>
	{
		virtual void AcceptPtr(IVisitorByPtr<BaseType>& visitor, const shared_ptr<BaseType>& thisptr)
		{ dynamic_cast<VisitorByPtr<BaseType, DerivedType>&>(visitor).Visit(shared_ptr<DerivedType>(thisptr, static_cast<DerivedType*>(this))); }

		virtual void AcceptPtr(IVisitorByPtr<const BaseType>& visitor, const shared_ptr<const BaseType>& thisptr) const
		{ dynamic_cast<VisitorByPtr<const BaseType, const DerivedType>&>(visitor).Visit(shared_ptr<const DerivedType>(thisptr, static_cast<const DerivedType*>(this))); }
	};

	template < typename BaseType, typename DerivedType >
	void ApplyVisitor(IVisitor<BaseType>& visitor, DerivedType& visitable)
	{ visitable.Accept(visitor); }

	template < typename BaseType, typename DerivedType >
	void ApplyVisitor(IVisitorByPtr<BaseType>& visitor, const shared_ptr<DerivedType>& visitable)
	{ visitable->AcceptPtr(visitor, shared_ptr<BaseType>(visitable)); }

}


#endif
