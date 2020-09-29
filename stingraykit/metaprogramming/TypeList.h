#ifndef STINGRAYKIT_METAPROGRAMMING_TYPELIST_H
#define STINGRAYKIT_METAPROGRAMMING_TYPELIST_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/metaprogramming/Functional.h>
#include <stingraykit/metaprogramming/TypeRelationships.h>

#include <sys/types.h>

namespace stingray
{

	struct TypeListEndNode
	{ };


	template < typename Value_, typename Next_ >
	struct TypeListNode
	{
		typedef Value_	ValueT;
		typedef Next_	Next;
	};


	const size_t TypeListNpos = static_cast<size_t>(-1);


	//////////////////////////////////////////////////////////////////////


	namespace Detail
	{
		template < typename T0, typename... Ts >
		struct IsTypeListEndNodes : public If<IsSame<T0, TypeListEndNode>::Value, IsTypeListEndNodes<Ts...>, FalseType>::ValueT
		{ };

		template < typename T0 >
		struct IsTypeListEndNodes<T0> : public FalseType
		{ };

		template < >
		struct IsTypeListEndNodes<TypeListEndNode> : public TrueType
		{ };

		template < typename... Ts >
		struct TypeListCreatorImpl;

		template < >
		struct TypeListCreatorImpl<>
		{
			typedef TypeListEndNode ValueT;
		};

		template < typename T0, typename... Ts >
		struct TypeListCreatorImpl<T0, Ts...>
		{
			typedef typename If<IsTypeListEndNodes<T0, Ts...>::Value, TypeListEndNode, TypeListNode<T0, typename TypeListCreatorImpl<Ts...>::ValueT> >::ValueT ValueT;
		};
	}

	template < typename... Ts > using TypeList = typename Detail::TypeListCreatorImpl<Ts...>::ValueT;

	//////////////////////////////////////////////////////////////////////


	namespace Detail
	{
		template < typename FirstTypeList, typename SecondTypeList >
		struct ListMerge
		{ typedef TypeListNode<typename FirstTypeList::ValueT, typename ListMerge<typename FirstTypeList::Next, SecondTypeList>::ValueT> ValueT; };

		template < typename SecondTypeList >
		struct ListMerge<TypeListEndNode, SecondTypeList>
		{ typedef SecondTypeList ValueT; };
	}

	template < typename TypeListOfTypeLists, typename ResultTypeList = TypeListEndNode >
	struct TypeListMerge
	{ typedef typename TypeListMerge<typename TypeListOfTypeLists::Next, typename Detail::ListMerge<ResultTypeList, typename TypeListOfTypeLists::ValueT>::ValueT>::ValueT ValueT; };

	template < typename ResultTypeList >
	struct TypeListMerge<TypeListEndNode, ResultTypeList>
	{ typedef ResultTypeList ValueT; };

	template < typename TypeList >
	struct GetTypeListLength
	{ static const size_t Value = GetTypeListLength<typename TypeList::Next>::Value + 1; };

	template < >
	struct GetTypeListLength<TypeListEndNode>
	{ static const size_t Value = 0; };

	template < typename TypeList, size_t N >
	struct GetTypeListItem
	{ typedef typename GetTypeListItem<typename TypeList::Next, N - 1>::ValueT	ValueT; };

	template < typename TypeList >
	struct GetTypeListItem<TypeList, 0>
	{ typedef typename TypeList::ValueT		ValueT; };

	template < typename TypeList >
	struct GetTypeListItem<TypeList, TypeListNpos>
	{ CompileTimeAssert<sizeof(TypeList) < 0>	ERROR_invalid_index; };

	template < typename TypeList, typename Val >
	struct TypeListAppend
	{ typedef TypeListNode<typename TypeList::ValueT, typename TypeListAppend<typename TypeList::Next, Val>::ValueT>	ValueT; };

	template < typename Val >
	struct TypeListAppend<TypeListEndNode, Val>
	{ typedef TypeListNode<Val, TypeListEndNode>	ValueT; };

	template < typename TypeList, typename Val >
	struct TypeListPrepend
	{ typedef TypeListNode<Val, TypeList>	ValueT; };

	template < typename Val >
	struct TypeListPrepend<TypeListEndNode, Val>
	{ typedef TypeListNode<Val, TypeListEndNode>	ValueT; };

	template < typename TypeList, size_t N >
	struct TryGetTypeListItem
	{ typedef typename TryGetTypeListItem<typename TypeList::Next, N - 1>::ValueT	ValueT; };

	template < size_t N >
	struct TryGetTypeListItem<TypeListEndNode, N>
	{ typedef TypeListEndNode				ValueT; };

	template < typename TypeList >
	struct TryGetTypeListItem<TypeList, 0>
	{ typedef typename TypeList::ValueT		ValueT; };

	template < >
	struct TryGetTypeListItem<TypeListEndNode, 0>
	{ typedef TypeListEndNode				ValueT; };

	template < typename TypeList >
	struct TryGetTypeListItem<TypeList, TypeListNpos>
	{ CompileTimeAssert<sizeof(TypeList) < 0>	ERROR_invalid_index; };

	template < typename TypeList, typename T, size_t Index_ = 0>
	struct IndexOfTypeListItem
	{
	private:
		static const size_t NextResult = IndexOfTypeListItem<typename TypeList::Next, T, Index_>::Value;
	public:
		static const size_t Value = (NextResult == TypeListNpos) ? TypeListNpos : (NextResult + 1);
	};

	template < typename T, size_t Index_ >
	struct IndexOfTypeListItem<TypeListEndNode, T, Index_> : integral_constant<size_t, TypeListNpos> { };

	template < typename TypeList, size_t Index_ >
	struct IndexOfTypeListItem<TypeList, typename TypeList::ValueT, Index_>
	{
	private:
		static const size_t NextResult = IndexOfTypeListItem<typename TypeList::Next, typename TypeList::ValueT, Index_ - 1>::Value;
	public:
		static const size_t Value = (NextResult == TypeListNpos) ? TypeListNpos : (NextResult + 1);
	};

	template < typename TypeList >
	struct IndexOfTypeListItem<TypeList, typename TypeList::ValueT, 0> : integral_constant<size_t, 0> { };

	template < typename TypeList, typename T >
	struct TypeListContains : integral_constant<bool, IndexOfTypeListItem<TypeList, T>::Value != TypeListNpos> { };


	template < typename TypeList, template<typename> class Predicate >
	struct EvaluateTypeListItem : integral_constant<bool, Predicate<typename TypeList::ValueT>::Value> { };

	template < template<typename> class Predicate >
	struct EvaluateTypeListItem<TypeListEndNode, Predicate> : FalseType { };

	template
		<
			typename TypeList,
			template <typename> class Predicate,
			bool CurrentIsOK = EvaluateTypeListItem<TypeList, Predicate>::Value
		>
	struct TypeListCopyIf;


	template < template <typename> class Predicate >
	struct TypeListCopyIf<TypeListEndNode, Predicate, true>		{ typedef TypeListEndNode	ValueT; };
	template < template <typename> class Predicate >
	struct TypeListCopyIf<TypeListEndNode, Predicate, false>	{ typedef TypeListEndNode	ValueT; };


	template < typename TypeList, template <typename> class Predicate >
	struct TypeListCopyIf<TypeList, Predicate, true>
	{ typedef TypeListNode<typename TypeList::ValueT, typename TypeListCopyIf<typename TypeList::Next, Predicate>::ValueT> ValueT; };

	template < typename TypeList, template <typename> class Predicate >
	struct TypeListCopyIf<TypeList, Predicate, false>
	{ typedef typename TypeListCopyIf<typename TypeList::Next, Predicate>::ValueT ValueT; };


	template < typename TypeList, template <typename, typename> class AccumulateFunc, typename InitialValue >
	struct TypeListAccumulate
	{ typedef typename AccumulateFunc<typename TypeList::ValueT, typename TypeListAccumulate<typename TypeList::Next, AccumulateFunc, InitialValue>::ValueT >::ValueT	ValueT; };

	template < template <typename, typename> class AccumulateFunc, typename InitialValue >
	struct TypeListAccumulate<TypeListEndNode, AccumulateFunc, InitialValue>
	{ typedef InitialValue	ValueT; };


	template < typename TypeList, template <typename> class Predicate >
	struct TypeListAllOf : integral_constant<bool, Predicate<typename TypeList::ValueT>::Value ? TypeListAllOf<typename TypeList::Next, Predicate>::Value : false> { };

	template < template <typename> class Predicate >
	struct TypeListAllOf<TypeListEndNode, Predicate> : TrueType { };


	template < typename TypeList, template <typename> class Predicate >
	struct TypeListAnyOf : integral_constant<bool, Predicate<typename TypeList::ValueT>::Value ? true : TypeListAnyOf<typename TypeList::Next, Predicate>::Value> { };

	template < template <typename> class Predicate >
	struct TypeListAnyOf<TypeListEndNode, Predicate> : FalseType { };


	template < typename TypeList, template <typename> class TransformFunc >
	struct TypeListTransform
	{ typedef TypeListNode<typename TransformFunc<typename TypeList::ValueT>::ValueT, typename TypeListTransform<typename TypeList::Next, TransformFunc>::ValueT> ValueT; };

	template < template <typename> class TransformFunc >
	struct TypeListTransform<TypeListEndNode, TransformFunc>
	{ typedef TypeListEndNode ValueT; };


	template < typename TypeList, template <typename> class Predicate >
	struct TypeListFindIf
	{ typedef typename If<Predicate<typename TypeList::ValueT>::Value, typename TypeList::ValueT, typename TypeListFindIf<typename TypeList::Next, Predicate>::ValueT>::ValueT ValueT; };

	template < template <typename> class Predicate >
	struct TypeListFindIf<TypeListEndNode, Predicate>
	{ typedef TypeListEndNode ValueT; };


	template < typename TypeList >
	struct TypeListReverse
	{ typedef typename TypeListAppend<typename TypeListReverse<typename TypeList::Next>::ValueT, typename TypeList::ValueT>::ValueT ValueT;};

	template < >
	struct TypeListReverse<TypeListEndNode>
	{ typedef TypeListEndNode ValueT; };


	template < size_t Count, template <size_t> class GeneratorFunctor, size_t Start = 0 >
	struct GenerateTypeList
	{ typedef TypeListNode<typename GeneratorFunctor<Start>::ValueT, typename GenerateTypeList<Count - 1, GeneratorFunctor, Start + 1>::ValueT> ValueT; };

	template < template <size_t> class GeneratorFunctor, size_t Start >
	struct GenerateTypeList<0, GeneratorFunctor, Start>
	{ typedef TypeListEndNode ValueT; };


	template < typename TypeList, template < typename, typename > class LessPredicate, typename Tail = typename TypeList::Next >
	struct MaxElement
	{
	private:
		typedef typename MaxElement<typename TypeList::Next, LessPredicate>::ValueT NextResult;

	public:
		typedef typename If<LessPredicate<typename TypeList::ValueT, NextResult>::Value, NextResult, typename TypeList::ValueT>::ValueT ValueT;
	};

	template < typename TypeList, template < typename, typename > class LessPredicate >
	struct MaxElement<TypeList, LessPredicate, TypeListEndNode>
	{ typedef typename TypeList::ValueT ValueT; };


	template < typename TypeList, template < typename, typename > class LessPredicate, typename Tail = typename TypeList::Next >
	struct MinElement
	{
	private:
		typedef typename MinElement<typename TypeList::Next, LessPredicate>::ValueT NextResult;

	public:
		typedef typename If<LessPredicate<typename TypeList::ValueT, NextResult>::Value, typename TypeList::ValueT, NextResult>::ValueT ValueT;
	};

	template < typename TypeList, template < typename, typename > class LessPredicate >
	struct MinElement<TypeList, LessPredicate, TypeListEndNode>
	{ typedef typename TypeList::ValueT ValueT; };


	template < typename TypeList_, template < typename, typename > class LessPredicate >
	struct TypeListSort
	{
		typedef typename TypeListMerge<TypeList<
				typename TypeListSort<typename TypeListCopyIf<typename TypeList_::Next, BindRight<LessPredicate, typename TypeList_::ValueT>::template ValueT>::ValueT, LessPredicate>::ValueT,
				TypeList<typename TypeList_::ValueT>,
				typename TypeListSort<typename TypeListCopyIf<typename TypeList_::Next, Not<BindRight<LessPredicate, typename TypeList_::ValueT>::template ValueT>::template ValueT>::ValueT, LessPredicate>::ValueT> >::ValueT ValueT;
	};

	template < template < typename, typename > class LessPredicate >
	struct TypeListSort<TypeListEndNode, LessPredicate>
	{ typedef TypeListEndNode ValueT; };


	namespace Detail
	{

		template < typename SortedTypeList, template <typename, typename, typename> class BranchType, typename LeafType, size_t Begin = 0, size_t End = GetTypeListLength<SortedTypeList>::Value >
		struct BalancedTypeTreeImpl
		{
		private:
			static const size_t Index = Begin + (End - Begin) / 2;

		public:
			typedef BranchType<
					typename GetTypeListItem<SortedTypeList, Index>::ValueT,
					typename BalancedTypeTreeImpl<SortedTypeList, BranchType, LeafType, Begin, Index>::ValueT,
					typename BalancedTypeTreeImpl<SortedTypeList, BranchType, LeafType, Index + 1, End>::ValueT
			> ValueT;
		};

		template < typename SortedTypeList, template <typename, typename, typename> class BranchType, typename LeafType, size_t Begin >
		struct BalancedTypeTreeImpl<SortedTypeList, BranchType, LeafType, Begin, Begin>
		{ typedef LeafType ValueT; };

	}

	template < typename TypeList, template <typename, typename> class LessPredicate, template <typename, typename, typename> class BranchType, typename LeafType >
	struct BalancedTypeTree
	{ typedef typename Detail::BalancedTypeTreeImpl<typename TypeListSort<TypeList, LessPredicate>::ValueT, BranchType, LeafType>::ValueT ValueT; };


	template < typename TypeList, template <typename> class FunctorClass >
	struct ForEachInTypeList
	{
		template < typename... Ts >
		static void Do(const Ts&... args)
		{
			FunctorClass<typename TypeList::ValueT>::Call(args...);
			ForEachInTypeList<typename TypeList::Next, FunctorClass>::Do(args...);
		}
	};

	template < template <typename> class FunctorClass >
	struct ForEachInTypeList<TypeListEndNode, FunctorClass>
	{
		template < typename... Ts >
		static void Do(const Ts&...) { }
	};


	namespace Detail
	{

		template < typename T, bool IsTypeList >
		struct ToTypeListImpl { typedef T	ValueT; };

		template < typename T >
		struct ToTypeListImpl<T, false> { typedef TypeList<T> ValueT; };

	}

	template < typename T >
	class ToTypeList
	{
	private:
		template < typename V, typename N>
		static YesType TestTypeList(const TypeListNode<V, N>*);
		static YesType TestTypeList(const TypeListEndNode*);
		static NoType TestTypeList(...);

	public:
		typedef typename Detail::ToTypeListImpl<T, sizeof(YesType) == sizeof(TestTypeList((const T*)0))>::ValueT		ValueT;
	};

}

#endif
