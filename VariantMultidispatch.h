#ifndef STINGRAY_TOOLKIT_VARIANTMULTIDISPATCH_H
#define STINGRAY_TOOLKIT_VARIANTMULTIDISPATCH_H


#include <stingray/toolkit/Tuple.h>
#include <stingray/toolkit/static_visitor.h>
#include <stingray/toolkit/variant.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_general_variants
	 * @{
	 */

	namespace Detail
	{
		template < typename Multivisitor_, typename VariantTypes_, typename ResolvedTypes_ >
		class MultiDispatchVisitor : public static_visitor<typename Multivisitor_::RetType>
		{
		public:
			typedef Tuple<VariantTypes_>	YetToResolve;
			typedef Tuple<ResolvedTypes_>	Resolved;

		private:
			Multivisitor_&		_multivisitor;
			YetToResolve		_yetToResolve;
			Resolved			_resolved;

		public:
			MultiDispatchVisitor(Multivisitor_& multivisitor, const YetToResolve& yetToResolve, const Resolved& resolved) :
				_multivisitor(multivisitor), _yetToResolve(yetToResolve), _resolved(resolved)
			{ }

			template < typename T >
			typename Multivisitor_::RetType operator() (T& val) const
			{
				typedef TypeListNode<T&, ResolvedTypes_>	NewResolvedTypes;
				return apply_visitor(
					MultiDispatchVisitor<Multivisitor_, typename VariantTypes_::Next, NewResolvedTypes>
						(_multivisitor, _yetToResolve.GetTail(), Tuple<NewResolvedTypes>(val, _resolved)), // Have to reverse the tuple in the end
					_yetToResolve.GetHead());
			}
		};

		template < typename Multivisitor_, typename ResolvedTypes_ >
		class MultiDispatchVisitor<Multivisitor_, TypeListEndNode, ResolvedTypes_> : public static_visitor<typename Multivisitor_::RetType>
		{
		public:
			typedef Tuple<TypeListEndNode>	YetToResolve;
			typedef Tuple<ResolvedTypes_>	Resolved;

		private:
			Multivisitor_&		_multivisitor;
			Resolved			_resolved;

		public:
			MultiDispatchVisitor(Multivisitor_& multivisitor, const YetToResolve& , const Resolved& resolved) :
				_multivisitor(multivisitor), _resolved(resolved)
			{ }

			template < typename T >
			typename Multivisitor_::RetType operator() (T& val) const
			{
				typedef TypeListNode<T&, ResolvedTypes_>	NewResolvedTypes;
				return FunctorInvoker::Invoke(_multivisitor, ReverseTuple(Tuple<NewResolvedTypes>(val, _resolved)));
			}
		};
	}


	template < typename Multivisitor_, typename VariantType1_, typename VariantType2_ >
	typename Multivisitor_::RetType Multidispatch(const Multivisitor_& multivisitor, const VariantType1_& var1, const VariantType2_& var2)
	{
		typedef TypeList_1<const VariantType2_&>	VariantTypes;
		return apply_visitor(Detail::MultiDispatchVisitor<const Multivisitor_, VariantTypes, TypeList_0>(multivisitor, Tuple<VariantTypes>(var2), Tuple<TypeList_0>()), var1);
	}

	/** @} */

}

#endif
