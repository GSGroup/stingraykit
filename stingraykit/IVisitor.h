#ifndef STINGRAYKIT_IVISITOR_H
#define STINGRAYKIT_IVISITOR_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/dynamic_caster.h>

namespace stingray
{

	namespace Detail
	{

		template < typename BaseType >
		struct IVisitorBase
		{
			virtual ~IVisitorBase() { }
		};

		template < typename BaseType, typename DerivedType >
		struct VisitorBase : public virtual IVisitorBase<BaseType>
		{
			virtual void InvokeVisit(DerivedType& visitable) = 0;
		};

		template < typename BaseType >
		struct IVisitorByPtrBase
		{
			virtual ~IVisitorByPtrBase() { }
		};

		template < typename BaseType, typename DerivedType >
		struct VisitorByPtrBase : public virtual IVisitorByPtrBase<BaseType>
		{
			virtual void InvokeVisit(const shared_ptr<DerivedType>& visitable) = 0;
		};

	}

	struct VisitorException : public Exception
	{
		template < typename VisitorType, typename VisitableType >
		VisitorException(const VisitorType& visitor, const VisitableType& visitable)
			: Exception(TypeInfo(visitor).GetName() + " can't visit " +  TypeInfo(visitable).GetName())
		{ }
	};

	template < typename BaseType, typename ValueType >
	struct IVisitor : public Detail::IVisitorBase<BaseType>
	{
	private:
		ValueType	_value;

	public:
		ValueType GetValue() const				{ return _value; }
		void SetValue(const ValueType& value)	{ _value = value; }
	};

	template < typename BaseType >
	struct IVisitor<BaseType, void> : public Detail::IVisitorBase<BaseType>
	{
		void GetValue() const { }
	};

	template < typename BaseType, typename DerivedType, typename ValueType = void >
	struct Visitor : public virtual IVisitor<BaseType, ValueType>, public Detail::VisitorBase<BaseType, DerivedType>
	{
		virtual void InvokeVisit(DerivedType& visitable)	{ this->SetValue(Visit(visitable)); }
		virtual ValueType Visit(DerivedType& visitable) = 0;
	};

	template < typename BaseType, typename DerivedType >
	struct Visitor<BaseType, DerivedType, void> : public virtual IVisitor<BaseType, void>, public Detail::VisitorBase<BaseType, DerivedType>
	{
		virtual void InvokeVisit(DerivedType& visitable)	{ Visit(visitable); }
		virtual void Visit(DerivedType& visitable) = 0;
	};

	template < typename BaseType >
	struct IVisitable
	{
		virtual ~IVisitable() { }

		virtual void Accept(Detail::IVisitorBase<BaseType>& visitor) = 0;
		virtual void Accept(Detail::IVisitorBase<const BaseType>& visitor) const = 0;
	};

	template < typename BaseType, typename DerivedType >
	struct Visitable : public virtual IVisitable<BaseType>
	{
		virtual void Accept(Detail::IVisitorBase<BaseType>& visitor)
		{
			Detail::VisitorBase<BaseType, DerivedType>* derivedVisitor = dynamic_caster(&visitor);
			STINGRAYKIT_CHECK(derivedVisitor, VisitorException(visitor, *this));
			derivedVisitor->InvokeVisit(*static_cast<DerivedType*>(this));
		}

		virtual void Accept(Detail::IVisitorBase<const BaseType>& visitor) const
		{
			Detail::VisitorBase<const BaseType, const DerivedType>* derivedVisitor = dynamic_caster(&visitor);
			STINGRAYKIT_CHECK(derivedVisitor, VisitorException(visitor, *this));
			derivedVisitor->InvokeVisit(*static_cast<const DerivedType*>(this));
		}
	};

	template < typename BaseType, typename ValueType >
	class IVisitorByPtr : public Detail::IVisitorByPtrBase<BaseType>
	{
	private:
		ValueType	_value;

	public:
		ValueType GetValue() const				{ return _value; }
		void SetValue(const ValueType& value)	{ _value = value; }
	};

	template < typename BaseType >
	struct IVisitorByPtr<BaseType, void> : public Detail::IVisitorByPtrBase<BaseType>
	{
		void GetValue() const { }
	};

	template < typename BaseType, typename DerivedType, typename ValueType = void >
	struct VisitorByPtr : public virtual IVisitorByPtr<BaseType, ValueType>, public Detail::VisitorByPtrBase<BaseType, DerivedType>
	{
		virtual void InvokeVisit(const shared_ptr<DerivedType>& visitable)	{ this->SetValue(Visit(visitable)); }

		virtual ValueType Visit(const shared_ptr<DerivedType>& visitable) = 0;
	};

	template < typename BaseType, typename DerivedType >
	struct VisitorByPtr<BaseType, DerivedType, void> : public virtual IVisitorByPtr<BaseType, void>, public Detail::VisitorByPtrBase<BaseType, DerivedType>
	{
		virtual void InvokeVisit(const shared_ptr<DerivedType>& visitable)	{ Visit(visitable); }

		virtual void Visit(const shared_ptr<DerivedType>& visitable) = 0;
	};

	template < typename BaseType >
	struct IVisitableByPtr
	{
		virtual ~IVisitableByPtr() { }

		virtual void AcceptPtr(Detail::IVisitorByPtrBase<BaseType>& visitor, const shared_ptr<BaseType>& thisptr) = 0;
		virtual void AcceptPtr(Detail::IVisitorByPtrBase<const BaseType>& visitor, const shared_ptr<const BaseType>& thisptr) const = 0;
	};

	template < typename BaseType, typename DerivedType >
	struct VisitableByPtr : public virtual IVisitableByPtr<BaseType>
	{
		virtual void AcceptPtr(Detail::IVisitorByPtrBase<BaseType>& visitor, const shared_ptr<BaseType>& thisptr)
		{
			Detail::VisitorByPtrBase<BaseType, DerivedType>* derivedVisitor = dynamic_caster(&visitor);
			STINGRAYKIT_CHECK(derivedVisitor, VisitorException(visitor, *this));
			derivedVisitor->InvokeVisit(shared_ptr<DerivedType>(thisptr, static_cast<DerivedType*>(this)));
		}

		virtual void AcceptPtr(Detail::IVisitorByPtrBase<const BaseType>& visitor, const shared_ptr<const BaseType>& thisptr) const
		{
			Detail::VisitorByPtrBase<const BaseType, const DerivedType>* derivedVisitor = dynamic_caster(&visitor);
			STINGRAYKIT_CHECK(derivedVisitor, VisitorException(visitor, *this));
			derivedVisitor->InvokeVisit(shared_ptr<const DerivedType>(thisptr, static_cast<const DerivedType*>(this)));
		}
	};


	template < typename BaseType, typename DerivedType, typename ValueType >
	ValueType ApplyVisitor(IVisitor<BaseType, ValueType>& visitor, DerivedType& visitable)
	{
		typedef typename Deconst<BaseType>::ValueT RawBaseType;
		typedef typename If<IsConst<DerivedType>::Value, const IVisitable<const RawBaseType>, IVisitable<RawBaseType> >::ValueT IVisitableBaseType;

		static_cast<IVisitableBaseType&>(visitable).Accept(visitor);
		return visitor.GetValue();
	}

	template < typename BaseType, typename DerivedType, typename ValueType >
	ValueType ApplyVisitor(IVisitorByPtr<BaseType, ValueType>& visitor, const shared_ptr<DerivedType>& visitable)
	{
		typedef typename Deconst<BaseType>::ValueT RawBaseType;
		typedef typename If<IsConst<DerivedType>::Value, const IVisitableByPtr<const RawBaseType>, IVisitableByPtr<RawBaseType> >::ValueT IVisitableByPtrBaseType;

		STINGRAYKIT_CHECK(visitable, NullArgumentException("visitable"));
		static_cast<IVisitableByPtrBaseType*>(visitable.get())->AcceptPtr(visitor, shared_ptr<BaseType>(visitable));
		return visitor.GetValue();
	}

}

#endif
