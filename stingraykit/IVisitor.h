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

		template < typename ValueType >
		class VisitContext;

		template < typename BaseType, typename ValueType >
		struct IVisitorBase
		{
			virtual ~IVisitorBase() { }

			virtual void InvokeVisit(typename GetVisitablePassingType<BaseType>::ValueT& visitable, VisitContext<ValueType>& context) const = 0;
		};

	}


	template < typename BaseType, typename ValueType = void >
	struct IVisitor : public virtual Detail::IVisitorBase<BaseType, ValueType>
	{
		virtual ~IVisitor() { }

		virtual ValueType Visit(typename Detail::GetVisitablePassingType<BaseType>::ValueT& visitable) const = 0;
	};


	template < typename BaseType, typename DerivedTypes, typename ValueType = void >
	struct Visitor;


	struct VisitorException : public Exception
	{
		template < typename VisitorType, typename VisitableType >
		VisitorException(const VisitorType& visitor, const VisitableType& visitable)
			: Exception(TypeInfo(visitor).GetName() + " can't visit " + TypeInfo(visitable).GetName())
		{ }
	};


	namespace Detail
	{

		template < typename BaseType, typename DerivedType, typename ValueType >
		struct VisitorImpl : public virtual IVisitor<BaseType, ValueType>
		{
			virtual ValueType Visit(typename GetVisitablePassingType<DerivedType>::ValueT& visitable) const = 0;
		};

		template < typename BaseType, typename ValueType >
		struct VisitorImpl<BaseType, BaseType, ValueType> : public virtual IVisitor<BaseType, ValueType>
		{
			virtual ValueType Visit(typename GetVisitablePassingType<BaseType>::ValueT& visitable) const
			{ STINGRAYKIT_THROW(VisitorException(*this, visitable)); }
		};

		template < typename BaseType, typename ValueType >
		struct ToVisitorImpl
		{
			template < typename DerivedType >
			struct type
			{ typedef VisitorImpl<BaseType, DerivedType, ValueType> ValueT; };
		};

		template < typename ValueType >
		class VisitContext
		{
		private:
			optional<ValueType>		_value;

		public:
			const ValueType& operator * () const
			{ return *_value; }

			template < typename BaseType, typename DerivedType >
			void Accept(const VisitorImpl<BaseType, DerivedType, ValueType>& visitor, typename GetVisitablePassingType<DerivedType>::ValueT& visitable)
			{ _value.emplace(visitor.Visit(visitable)); }
		};

		template < >
		class VisitContext<void>
		{
		public:
			void operator * () const
			{ }

			template < typename BaseType, typename DerivedType >
			void Accept(const VisitorImpl<BaseType, DerivedType, void>& visitor, typename GetVisitablePassingType<DerivedType>::ValueT& visitable)
			{ visitor.Visit(visitable); }
		};

		template < typename BaseType >
		struct IVisitorWrapper
		{
			virtual ~IVisitorWrapper() { }

			virtual void VisitWrapped(typename GetVisitablePassingType<BaseType>::ValueT& visitable) const = 0;
		};

		template < typename BaseType, typename DerivedType >
		struct VisitorWrapperBase : public virtual IVisitorWrapper<BaseType>
		{
			virtual void VisitWrapped(typename GetVisitablePassingType<DerivedType>::ValueT& visitable) const = 0;
		};

		template < typename BaseType, typename DerivedType, typename ValueType >
		class VisitorWrapperImpl : public VisitorWrapperBase<BaseType, DerivedType>
		{
		private:
			const VisitorImpl<BaseType, DerivedType, ValueType>&	_visitor;
			VisitContext<ValueType>&								_context;

		public:
			VisitorWrapperImpl(const VisitorImpl<BaseType, DerivedType, ValueType>& visitor, VisitContext<ValueType>& context) : _visitor(visitor), _context(context) { }

			virtual void VisitWrapped(typename GetVisitablePassingType<DerivedType>::ValueT& visitable) const
			{ _context.Accept(_visitor, visitable); }
		};

		template < typename BaseType, typename ValueType >
		struct ToVisitorWrapperImpl
		{
			template < typename DerivedType >
			struct type
			{ typedef VisitorWrapperImpl<BaseType, DerivedType, ValueType> ValueT; };
		};

		template < typename BaseType, typename DerivedTypes, typename ValueType >
		struct GetVisitorWrapperBase
		{
			typedef typename InheritanceAccumulator<
					typename TypeListTransform<
							typename TypeListAppend<typename ToTypeList<DerivedTypes>::ValueT, BaseType>::ValueT,
							Detail::ToVisitorWrapperImpl<BaseType, ValueType>::template type
					>::ValueT>::ValueT ValueT;
		};

		template < typename BaseType, typename DerivedTypes, typename ValueType >
		struct VisitorWrapper : public GetVisitorWrapperBase<BaseType, DerivedTypes, ValueType>::ValueT
		{
			VisitorWrapper(const Visitor<BaseType, DerivedTypes, ValueType>& visitor, VisitContext<ValueType>& context)
				:	GetVisitorWrapperBase<BaseType, DerivedTypes, ValueType>::ValueT(visitor, context)
			{ }
		};

	}


	template < typename BaseType, typename DerivedTypes, typename ValueType >
	struct VisitorBase : public InheritanceAccumulator<
			typename TypeListTransform<
					typename TypeListAppend<typename ToTypeList<DerivedTypes>::ValueT, BaseType>::ValueT,
					Detail::ToVisitorImpl<BaseType, ValueType>::template type
			>::ValueT>::ValueT
	{
		typedef ValueType RetType;
	};


	template < typename BaseType >
	struct IVisitable : public virtual IVisitable<const BaseType>
	{
		virtual void Accept(const Detail::IVisitorWrapper<BaseType>& visitor, BaseType& baseThis) = 0;
		virtual void Accept(const Detail::IVisitorWrapper<shared_ptr<BaseType> >& visitor, const shared_ptr<BaseType>& baseThis) = 0;
	};


	template < typename BaseType >
	struct IVisitable<const BaseType>
	{
		virtual ~IVisitable() { }

		virtual void Accept(const Detail::IVisitorWrapper<const BaseType>& visitor, const BaseType& baseThis) const = 0;
		virtual void Accept(const Detail::IVisitorWrapper<shared_ptr<const BaseType> >& visitor, const shared_ptr<const BaseType>& baseThis) const = 0;
	};


	template < typename BaseType, typename DerivedType >
	struct Visitable : public virtual IVisitable<BaseType>, public Visitable<const BaseType, const DerivedType>
	{
		virtual void Accept(const Detail::IVisitorWrapper<BaseType>& visitor, BaseType& baseThis)
		{
			if (const Detail::VisitorWrapperBase<BaseType, DerivedType>* const derivedVisitor = dynamic_caster(&visitor))
				derivedVisitor->VisitWrapped(*static_cast<DerivedType*>(this));
			else
				visitor.VisitWrapped(baseThis);
		}

		virtual void Accept(const Detail::IVisitorWrapper<shared_ptr<BaseType> >& visitor, const shared_ptr<BaseType>& baseThis)
		{
			if (const Detail::VisitorWrapperBase<shared_ptr<BaseType>, shared_ptr<DerivedType> >* const derivedVisitor = dynamic_caster(&visitor))
				derivedVisitor->VisitWrapped(shared_ptr<DerivedType>(baseThis, static_cast<DerivedType*>(this)));
			else
				visitor.VisitWrapped(baseThis);
		}
	};


	template < typename BaseType, typename DerivedType >
	struct Visitable<const BaseType, const DerivedType> : public virtual IVisitable<const BaseType>
	{
		virtual void Accept(const Detail::IVisitorWrapper<const BaseType>& visitor, const BaseType& baseThis) const
		{
			if (const Detail::VisitorWrapperBase<const BaseType, const DerivedType>* const derivedVisitor = dynamic_caster(&visitor))
				derivedVisitor->VisitWrapped(*static_cast<const DerivedType*>(this));
			else
				visitor.VisitWrapped(baseThis);
		}

		virtual void Accept(const Detail::IVisitorWrapper<shared_ptr<const BaseType> >& visitor, const shared_ptr<const BaseType>& baseThis) const
		{
			if (const Detail::VisitorWrapperBase<shared_ptr<const BaseType>, shared_ptr<const DerivedType> >* const derivedVisitor = dynamic_caster(&visitor))
				derivedVisitor->VisitWrapped(shared_ptr<const DerivedType>(baseThis, static_cast<const DerivedType*>(this)));
			else
				visitor.VisitWrapped(baseThis);
		}
	};


	template < typename BaseType, typename DerivedTypes, typename ValueType >
	struct Visitor : public VisitorBase<BaseType, DerivedTypes, ValueType>
	{
		virtual void InvokeVisit(BaseType& visitable, Detail::VisitContext<ValueType>& context) const
		{ static_cast<typename If<IsConst<BaseType>::Value, const IVisitable<BaseType>, IVisitable<BaseType> >::ValueT&>(visitable).Accept(Detail::VisitorWrapper<BaseType, DerivedTypes, ValueType>(*this, context), visitable); }
	};


	template < typename BaseType, typename DerivedTypes, typename ValueType >
	struct Visitor<shared_ptr<BaseType>, DerivedTypes, ValueType> : public VisitorBase<shared_ptr<BaseType>, DerivedTypes, ValueType>
	{
		virtual void InvokeVisit(const shared_ptr<BaseType>& visitable, Detail::VisitContext<ValueType>& context) const
		{ static_cast<typename If<IsConst<BaseType>::Value, const IVisitable<BaseType>, IVisitable<BaseType> >::ValueT&>(*visitable).Accept(Detail::VisitorWrapper<shared_ptr<BaseType>, DerivedTypes, ValueType>(*this, context), visitable); }
	};


	template < typename VisitorType >
	class VisitorApplyFunc : public function_info<typename VisitorType::RetType, UnspecifiedParamTypes>
	{
	private:
		VisitorType		_visitor;

	public:
		explicit VisitorApplyFunc(const VisitorType& visitor) : _visitor(visitor) { }

		template < typename DerivedType >
		typename VisitorType::RetType operator () (DerivedType& visitable) const
		{
			Detail::VisitContext<typename VisitorType::RetType> context;
			_visitor.InvokeVisit(visitable, context);
			return *context;
		}
	};


	template < typename VisitorType >
	VisitorApplyFunc<VisitorType> MakeVisitorApplier(const VisitorType& visitor)
	{ return VisitorApplyFunc<VisitorType>(visitor); }


	template < typename VisitorType, typename DerivedType >
	typename VisitorType::RetType ApplyVisitor(const VisitorType& visitor, DerivedType& visitable)
	{ return MakeVisitorApplier(visitor)(visitable); }

}

#endif
