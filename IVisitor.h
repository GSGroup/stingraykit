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
	struct VisitorBase : public virtual IVisitor<BaseType>
	{
		virtual void InvokeVisit(DerivedType& visitable) = 0;
	};

	template < typename BaseType, typename ValueType >
	struct IVisitorWithValue : public IVisitor<BaseType>
	{
	private:
		ValueType	_value;

	public:
		ValueType GetValue() const				{ return _value; }
		void SetValue(const ValueType& value)	{ _value = value; }
	};

	template < typename BaseType, typename DerivedType, typename ValueType = void >
	struct Visitor : public virtual IVisitorWithValue<BaseType, ValueType>, public VisitorBase<BaseType, DerivedType>
	{
		virtual void InvokeVisit(DerivedType& visitable)	{ SetValue(Visit(visitable)); }
		virtual ValueType Visit(DerivedType& visitable) = 0;
	};

	template < typename BaseType, typename DerivedType >
	struct Visitor<BaseType, DerivedType, void> : public VisitorBase<BaseType, DerivedType>
	{
		virtual void InvokeVisit(DerivedType& visitable)	{ Visit(visitable); }
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
		{ dynamic_cast<VisitorBase<BaseType, DerivedType>&>(visitor).InvokeVisit(*static_cast<DerivedType*>(this)); }

		virtual void Accept(IVisitor<const BaseType>& visitor) const
		{ dynamic_cast<VisitorBase<const BaseType, const DerivedType>&>(visitor).InvokeVisit(*static_cast<const DerivedType*>(this)); }
	};

	template < typename BaseType >
	struct IVisitorByPtr
	{
		virtual ~IVisitorByPtr() { }
	};

	template < typename BaseType, typename DerivedType >
	struct VisitorByPtrBase : public virtual IVisitorByPtr<BaseType>
	{
		virtual void InvokeVisit(const shared_ptr<DerivedType>& visitable) = 0;
	};

	template < typename BaseType, typename ValueType >
	class IVisitorByPtrWithValue : public IVisitorByPtr<BaseType>
	{
	private:
		ValueType	_value;

	public:
		ValueType GetValue() const				{ return _value; }
		void SetValue(const ValueType& value)	{ _value = value; }
	};

	template < typename BaseType, typename DerivedType, typename ValueType = void >
	struct VisitorByPtr : public virtual IVisitorByPtrWithValue<BaseType, ValueType>, public VisitorByPtrBase<BaseType, DerivedType>
	{
		virtual void InvokeVisit(const shared_ptr<DerivedType>& visitable)	{ SetValue(Visit(visitable)); }

		virtual ValueType Visit(const shared_ptr<DerivedType>& visitable) = 0;
	};

	template < typename BaseType, typename DerivedType >
	struct VisitorByPtr<BaseType, DerivedType, void> : public VisitorByPtrBase<BaseType, DerivedType>
	{
		virtual void InvokeVisit(const shared_ptr<DerivedType>& visitable)	{ Visit(visitable); }

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
		{ dynamic_cast<VisitorByPtrBase<BaseType, DerivedType>&>(visitor).InvokeVisit(shared_ptr<DerivedType>(thisptr, static_cast<DerivedType*>(this))); }

		virtual void AcceptPtr(IVisitorByPtr<const BaseType>& visitor, const shared_ptr<const BaseType>& thisptr) const
		{ dynamic_cast<VisitorByPtrBase<const BaseType, const DerivedType>&>(visitor).InvokeVisit(shared_ptr<const DerivedType>(thisptr, static_cast<const DerivedType*>(this))); }
	};

	template < typename BaseType, typename DerivedType >
	void ApplyVisitor(IVisitor<BaseType>& visitor, DerivedType& visitable)
	{ visitable.Accept(visitor); }

	template < typename BaseType, typename DerivedType, typename ValueType >
	ValueType ApplyVisitor(IVisitorWithValue<BaseType, ValueType>& visitor, DerivedType& visitable)
	{
		visitable.Accept(visitor);
		return visitor.GetValue();
	}

	template < typename BaseType, typename DerivedType >
	void ApplyVisitor(IVisitorByPtr<BaseType>& visitor, const shared_ptr<DerivedType>& visitable)
	{ visitable->AcceptPtr(visitor, shared_ptr<BaseType>(visitable)); }

	template < typename BaseType, typename DerivedType, typename ValueType >
	ValueType ApplyVisitor(IVisitorByPtrWithValue<BaseType, ValueType>& visitor, const shared_ptr<DerivedType>& visitable)
	{
		visitable->AcceptPtr(visitor, shared_ptr<BaseType>(visitable));
		return visitor.GetValue();
	}

}


#endif
