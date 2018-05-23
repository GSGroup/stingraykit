#ifndef STINGRAYKIT_IVISITOR_H
#define STINGRAYKIT_IVISITOR_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/dynamic_caster.h>
#include <stingraykit/optional.h>

namespace stingray
{

	namespace Detail
	{

		template < typename T >
		struct GetVisitablePassingType
		{ typedef T ValueT; };

		template < typename T >
		struct GetVisitablePassingType<shared_ptr<T> >
		{ typedef const shared_ptr<T> ValueT; };

		template < typename BaseType >
		struct IVisitorBase
		{
			virtual ~IVisitorBase() { }

			virtual void InvokeVisit(typename GetVisitablePassingType<BaseType>::ValueT& visitable) = 0;
		};

		template < typename BaseType, typename DerivedType >
		struct VisitorBase : public virtual IVisitorBase<BaseType>
		{
			virtual void InvokeVisit(typename GetVisitablePassingType<DerivedType>::ValueT& visitable) = 0;
		};

	}


	struct VisitorException : public Exception
	{
		template < typename VisitorType, typename VisitableType >
		VisitorException(const VisitorType& visitor, const VisitableType& visitable) :
			Exception(TypeInfo(visitor).GetName() + " can't visit " + TypeInfo(visitable).GetName())
		{ }
	};


	template < typename BaseType, typename ValueType >
	struct IVisitor : public Detail::IVisitorBase<BaseType>
	{
	private:
		optional<ValueType>		_value;

	public:
		ValueType GetValue() const						{ return *_value; }
		void SetValue(const ValueType& value)			{ _value = value; }

		IVisitor<BaseType, ValueType>& GetReference()	{ return *this; }
	};


	template < typename BaseType >
	struct IVisitor<BaseType, void> : public Detail::IVisitorBase<BaseType>
	{
		void GetValue() const							{ }

		IVisitor<BaseType, void>& GetReference()		{ return *this; }
	};


	namespace Detail
	{

		template < typename BaseType, typename DerivedType, typename ValueType >
		struct VisitorImplBase : public VisitorBase<BaseType, DerivedType>
		{
			virtual ValueType Visit(typename GetVisitablePassingType<DerivedType>::ValueT& visitable) = 0;
		};

		template < typename BaseType, typename ValueType >
		struct VisitorImplBase<BaseType, BaseType, ValueType> : public virtual IVisitorBase<BaseType>
		{
			virtual ValueType Visit(typename GetVisitablePassingType<BaseType>::ValueT& visitable)
			{ STINGRAYKIT_THROW(VisitorException(*this, visitable)); }
		};

		template < typename BaseType, typename DerivedType, typename ValueType >
		struct VisitorImpl : public virtual IVisitor<BaseType, ValueType>, public VisitorImplBase<BaseType, DerivedType, ValueType>
		{
			virtual void InvokeVisit(typename GetVisitablePassingType<DerivedType>::ValueT& visitable)
			{ this->SetValue(this->Visit(visitable)); }
		};

		template < typename BaseType, typename DerivedType >
		struct VisitorImpl<BaseType, DerivedType, void> : public virtual IVisitor<BaseType, void>, public VisitorImplBase<BaseType, DerivedType, void>
		{
			virtual void InvokeVisit(typename GetVisitablePassingType<DerivedType>::ValueT& visitable)
			{ this->Visit(visitable); }
		};

		template < typename BaseType, typename ValueType >
		struct ToVisitorImpl
		{
			template < typename DerivedType >
			struct type
			{
				typedef VisitorImpl<BaseType, DerivedType, ValueType> ValueT;
			};
		};

		template < typename BaseType >
		struct EndNode { };

	}


	template < typename BaseType, typename DerivedTypes, typename ValueType = void >
	struct Visitor : public InheritanceAccumulator<
			typename TypeListTransform<
					typename TypeListAppend<typename ToTypeList<DerivedTypes>::ValueT, BaseType>::ValueT,
					Detail::ToVisitorImpl<BaseType, ValueType>::template type>::ValueT,
			Detail::EndNode<BaseType> >::ValueT
	{ };


	template < typename BaseType >
	struct IVisitable
	{
		virtual ~IVisitable() { }

		virtual void Accept(Detail::IVisitorBase<BaseType>& visitor) = 0;
		virtual void Accept(Detail::IVisitorBase<const BaseType>& visitor) const = 0;

		virtual void Accept(Detail::IVisitorBase<shared_ptr<BaseType> >& visitor, const shared_ptr<BaseType>& thisptr) = 0;
		virtual void Accept(Detail::IVisitorBase<shared_ptr<const BaseType> >& visitor, const shared_ptr<const BaseType>& thisptr) const = 0;
	};


	template < typename BaseType, typename DerivedType >
	struct Visitable : public virtual IVisitable<BaseType>
	{
		virtual void Accept(Detail::IVisitorBase<BaseType>& visitor)
		{
			DerivedType* derivedThis = static_cast<DerivedType*>(this);
			Detail::VisitorBase<BaseType, DerivedType>* derivedVisitor = dynamic_caster(&visitor);
			if (derivedVisitor)
				derivedVisitor->InvokeVisit(*derivedThis);
			else
				visitor.InvokeVisit(*static_cast<BaseType*>(derivedThis));
		}

		virtual void Accept(Detail::IVisitorBase<const BaseType>& visitor) const
		{
			const DerivedType* derivedThis = static_cast<const DerivedType*>(this);
			Detail::VisitorBase<const BaseType, const DerivedType>* derivedVisitor = dynamic_caster(&visitor);
			if (derivedVisitor)
				derivedVisitor->InvokeVisit(*derivedThis);
			else
				visitor.InvokeVisit(*static_cast<const BaseType*>(derivedThis));
		}

		virtual void Accept(Detail::IVisitorBase<shared_ptr<BaseType> >& visitor, const shared_ptr<BaseType>& thisptr)
		{
			DerivedType* derivedThis = static_cast<DerivedType*>(this);
			Detail::VisitorBase<shared_ptr<BaseType>, shared_ptr<DerivedType> >* derivedVisitor = dynamic_caster(&visitor);
			if (derivedVisitor)
				derivedVisitor->InvokeVisit(shared_ptr<DerivedType>(thisptr, derivedThis));
			else
				visitor.InvokeVisit(thisptr);
		}

		virtual void Accept(Detail::IVisitorBase<shared_ptr<const BaseType> >& visitor, const shared_ptr<const BaseType>& thisptr) const
		{
			const DerivedType* derivedThis = static_cast<const DerivedType*>(this);
			Detail::VisitorBase<shared_ptr<const BaseType>, shared_ptr<const DerivedType> >* derivedVisitor = dynamic_caster(&visitor);
			if (derivedVisitor)
				derivedVisitor->InvokeVisit(shared_ptr<const DerivedType>(thisptr, derivedThis));
			else
				visitor.InvokeVisit(thisptr);
		}
	};


	template < typename BaseType, typename DerivedType, typename ValueType >
	typename EnableIf<!IsSharedPtr<BaseType>::Value, ValueType>::ValueT ApplyVisitor(IVisitor<BaseType, ValueType>& visitor, DerivedType& visitable)
	{
		typedef typename RemoveConst<BaseType>::ValueT RawBaseType;
		typedef typename If<IsConst<DerivedType>::Value, const IVisitable<const RawBaseType>, IVisitable<RawBaseType> >::ValueT IVisitableBaseType;

		static_cast<IVisitableBaseType&>(visitable).Accept(visitor);
		return visitor.GetValue();
	}


	template < typename BaseType, typename DerivedType, typename ValueType >
	ValueType ApplyVisitor(IVisitor<shared_ptr<BaseType> , ValueType>& visitor, const shared_ptr<DerivedType>& visitable)
	{
		typedef typename RemoveConst<BaseType>::ValueT RawBaseType;
		typedef typename If<IsConst<DerivedType>::Value, const IVisitable<const RawBaseType>, IVisitable<RawBaseType> >::ValueT IVisitableBaseType;

		STINGRAYKIT_CHECK(visitable, NullArgumentException("visitable"));
		static_cast<IVisitableBaseType&>(*visitable).Accept(visitor, visitable);
		return visitor.GetValue();
	}


	template < typename BaseType, typename ValueType >
	class VisitorApplyHelper : public function_info<ValueType, UnspecifiedParamTypes>
	{
	private:
		IVisitor<BaseType, ValueType>&	_visitor;

	public:
		explicit VisitorApplyHelper(IVisitor<BaseType, ValueType>& visitor) : _visitor(visitor) { }

		template < typename DerivedType >
		ValueType operator () (const DerivedType& visitable) const
		{ return ApplyVisitor(_visitor, visitable); }

		template < typename DerivedType >
		ValueType operator () (const shared_ptr<DerivedType>& visitable) const
		{ return ApplyVisitor(_visitor, visitable); }
	};


	template < typename BaseType, typename ValueType >
	VisitorApplyHelper<BaseType, ValueType> MakeVisitorApplier(IVisitor<BaseType, ValueType>& visitor)
	{ return VisitorApplyHelper<BaseType, ValueType>(visitor); }

}

#endif
