#ifndef STINGRAY_TOOLKIT_TYPELIST_H
#define STINGRAY_TOOLKIT_TYPELIST_H


#include <sys/types.h>

#include <stingray/toolkit/Macro.h>
#include <stingray/toolkit/MetaProgramming.h>


namespace stingray
{


	struct TypeListEndNode
	{ typedef TypeListEndNode type; };


	template < typename Value_, typename Next_ >
	struct TypeListNode
	{
		typedef Value_	ValueT;
		typedef Next_	Next;
	};


	//////////////////////////////////////////////////////////////////////


#define TY typename

	typedef TypeListEndNode TypeList_0;

	template < TY T1 >
	struct TypeList_1 : public TypeListNode<T1, TypeList_0>
	{ typedef TypeListNode<T1, TypeList_0> type; };


#define MAX_TYPELIST_LEN 50

#define DETAIL_TYPELIST_PARAMS_DECL(Index_, Default_) TOOLKIT_COMMA_IF(Index_) typename TOOLKIT_CAT(T, Index_) Default_
#define DETAIL_TYPELIST_PARAMS_USAGE(Index_, Shift_) TOOLKIT_COMMA_IF(Index_) TOOLKIT_CAT(T, TOOLKIT_ADD(Index_, Shift_))
#define DETAIL_TYPELISTENDNODE(Index_, unused) TOOLKIT_COMMA_IF(Index_) TypeListEndNode

	namespace Detail
	{
		template< TOOLKIT_REPEAT(MAX_TYPELIST_LEN, DETAIL_TYPELIST_PARAMS_DECL, TOOLKIT_EMPTY()) >
		struct TypeListCreatorImpl
		{
			typedef TypeListNode<T0, typename TypeListCreatorImpl<TOOLKIT_REPEAT(TOOLKIT_DEC(MAX_TYPELIST_LEN), DETAIL_TYPELIST_PARAMS_USAGE, 1), TypeListEndNode>::ValueT > ValueT;
		};

		template<>
		struct TypeListCreatorImpl<TOOLKIT_REPEAT(MAX_TYPELIST_LEN, DETAIL_TYPELISTENDNODE, ~)>
		{
			typedef TypeListEndNode ValueT;
		};
	}

	template< TOOLKIT_REPEAT(MAX_TYPELIST_LEN, DETAIL_TYPELIST_PARAMS_DECL, = TypeListEndNode) >
	struct TypeList : public Detail::TypeListCreatorImpl< TOOLKIT_REPEAT(MAX_TYPELIST_LEN, DETAIL_TYPELIST_PARAMS_USAGE, 0) >::ValueT
	{
		typedef typename Detail::TypeListCreatorImpl< TOOLKIT_REPEAT(MAX_TYPELIST_LEN, DETAIL_TYPELIST_PARAMS_USAGE, 0) >::ValueT type;
	};

#define DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(Size_) \
	template< TOOLKIT_REPEAT(Size_, DETAIL_TYPELIST_PARAMS_DECL, TOOLKIT_EMPTY()) > \
	struct TypeList_##Size_ : \
		public TypeListNode<T0, TOOLKIT_CAT(TypeList_, TOOLKIT_DEC(Size_)) <TOOLKIT_REPEAT(TOOLKIT_DEC(Size_), DETAIL_TYPELIST_PARAMS_USAGE, 1)> > \
	{ typedef TypeListNode<T0, TOOLKIT_CAT(TypeList_, TOOLKIT_DEC(Size_)) <TOOLKIT_REPEAT(TOOLKIT_DEC(Size_), DETAIL_TYPELIST_PARAMS_USAGE, 1)> > type; };

	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(2)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(3)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(4)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(5)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(6)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(7)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(8)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(9)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(10)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(11)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(12)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(13)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(14)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(15)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(16)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(17)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(18)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(19)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(20)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(21)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(22)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(23)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(24)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(25)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(26)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(27)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(28)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(29)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(30)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(31)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(32)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(33)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(34)
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(35)

#undef DETAIL_TYPELISTENDNODE
#undef DETAIL_TYPELIST_PARAMS_USAGE
#undef DETAIL_TYPELIST_PARAMS_DECL

/*
	template < TY T1, TY T2 >
	struct TypeList_2 : public TypeListNode<T1, TypeList_1<T2> > { };

	template < TY T1, TY T2, TY T3 >
	struct TypeList_3 : public TypeListNode<T1, TypeList_2<T2, T3> > { };

	template < TY T1, TY T2, TY T3, TY T4 >
	struct TypeList_4 : public TypeListNode<T1, TypeList_3<T2, T3, T4> > { };

	template < TY T1, TY T2, TY T3, TY T4, TY T5 >
	struct TypeList_5 : public TypeListNode<T1, TypeList_4<T2, T3, T4, T5> > { };

	template < TY T1, TY T2, TY T3, TY T4, TY T5, TY T6 >
	struct TypeList_6 : public TypeListNode<T1, TypeList_5<T2, T3, T4, T5, T6> > { };

	template < TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7 >
	struct TypeList_7 : public TypeListNode<T1, TypeList_6<T2, T3, T4, T5, T6, T7> > { };

	template < TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8 >
	struct TypeList_8 : public TypeListNode<T1, TypeList_7<T2, T3, T4, T5, T6, T7, T8> > { };

	template < TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9 >
	struct TypeList_9 : public TypeListNode<T1, TypeList_8<T2, T3, T4, T5, T6, T7, T8, T9> > { };

	template < TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10 >
	struct TypeList_10 : public TypeListNode<T1, TypeList_9<T2, T3, T4, T5, T6, T7, T8, T9, T10> > { };
*/


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
	struct GetTypeListItem<TypeList, -1>
	{ CompileTimeAssert<sizeof(TypeList) < 0>	ERROR_invalid_index; };

	template < typename TypeList, typename Val >
	struct TypeListAppend
	{ typedef TypeListNode<typename TypeList::ValueT, typename TypeListAppend<typename TypeList::Next, Val>::ValueT>	ValueT; };

	template < typename Val >
	struct TypeListAppend<TypeListEndNode, Val>
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
	struct TryGetTypeListItem<TypeList, -1>
	{ CompileTimeAssert<sizeof(TypeList) < 0>	ERROR_invalid_index; };

	template < typename TypeList, typename T, size_t Index_ = 0>
	struct IndexOfTypeListItem
	{
	private:
		static const int NextResult = IndexOfTypeListItem<typename TypeList::Next, T, Index_>::Value;
	public:
		static const int Value = (NextResult == -1) ? -1 : (NextResult + 1);
	};

	template < typename TypeList, typename T >
	struct TypeListContains
	{
		static const bool Value = IndexOfTypeListItem<TypeList, T>::Value != -1;
	};

	template < typename T, size_t Index_ >
	struct IndexOfTypeListItem<TypeListEndNode, T, Index_> { static const int Value = -1; };

	template < typename TypeList, size_t Index_ >
	struct IndexOfTypeListItem<TypeList, typename TypeList::ValueT, Index_>
	{
	private:
		static const int NextResult = IndexOfTypeListItem<typename TypeList::Next, typename TypeList::ValueT, Index_ - 1>::Value;
	public:
		static const int Value = (NextResult == -1) ? -1 : (NextResult + 1);
	};

	template < typename TypeList >
	struct IndexOfTypeListItem<TypeList, typename TypeList::ValueT, 0> { static const int Value = 0; };


	template < typename TypeList, template<typename> class Predicate >
	struct EvaluateTypeListItem
	{ static const bool Value = Predicate<typename TypeList::ValueT>::Value; };

	template < template<typename> class Predicate >
	struct EvaluateTypeListItem<TypeListEndNode, Predicate>
	{ static const bool Value = false; };

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


	template < unsigned Count, template <int> class GeneratorFunctor, int Start = 0 >
	struct GenerateTypeList
	{ typedef TypeListNode<typename GeneratorFunctor<Start>::ValueT, typename GenerateTypeList<Count - 1, GeneratorFunctor, Start + 1>::ValueT> ValueT; };

	template < template <int> class GeneratorFunctor, int Start >
	struct GenerateTypeList<0, GeneratorFunctor, Start>
	{ typedef TypeListEndNode ValueT; };


	template<typename TypeList, template<typename, typename> class LessPredicate, typename Tail = typename TypeList::Next>
	struct MaxElement
	{
	private:
		typedef typename MaxElement<typename TypeList::Next, LessPredicate>::ValueT NextResult;

	public:
		typedef typename If<LessPredicate<typename TypeList::ValueT, NextResult>::Value, NextResult, typename TypeList::ValueT>::ValueT ValueT;
	};

	template<typename TypeList, template<typename, typename> class LessPredicate>
	struct MaxElement<TypeList, LessPredicate, TypeListEndNode>
	{ typedef typename TypeList::ValueT ValueT; };


	template<typename TypeList, template<typename, typename> class LessPredicate, typename Tail = typename TypeList::Next>
	struct MinElement
	{
	private:
		typedef typename MinElement<typename TypeList::Next, LessPredicate>::ValueT NextResult;

	public:
		typedef typename If<LessPredicate<typename TypeList::ValueT, NextResult>::Value, typename TypeList::ValueT, NextResult>::ValueT ValueT;
	};

	template<typename TypeList, template<typename, typename> class LessPredicate>
	struct MinElement<TypeList, LessPredicate, TypeListEndNode>
	{ typedef typename TypeList::ValueT ValueT; };


	template <typename TypeList, template<typename, typename> class LessPredicate>
	struct TypeListSort
	{
		typedef typename TypeListMerge<TypeList_3<
				typename TypeListSort<typename TypeListCopyIf<typename TypeList::Next, BindRight<LessPredicate, typename TypeList::ValueT>::template ValueT>::ValueT, LessPredicate>::ValueT,
				TypeList_1<typename TypeList::ValueT>,
				typename TypeListSort<typename TypeListCopyIf<typename TypeList::Next, Not<BindRight<LessPredicate, typename TypeList::ValueT>::template ValueT>::template ValueT>::ValueT, LessPredicate>::ValueT> >::ValueT ValueT;
	};

	template <template<typename, typename> class LessPredicate>
	struct TypeListSort<TypeListEndNode, LessPredicate>
	{ typedef TypeListEndNode ValueT; };


	template < typename TypeList, template <typename> class FunctorClass >
	struct ForEachInTypeList
	{
		static void Do()
		{
			FunctorClass<typename TypeList::ValueT>::Call();
			ForEachInTypeList<typename TypeList::Next, FunctorClass>::Do();
		}

#define DETAIL_TOOLKIT_DECLARE_FOREACHINTYPELIST_DO(TypesDecl_, TypesUsage_, ParamsDecl_, ParamsUsage_) \
		template < TypesDecl_ > \
		static void Do(ParamsDecl_) \
		{ \
			FunctorClass<typename TypeList::ValueT>::Call(ParamsUsage_); \
			ForEachInTypeList<typename TypeList::Next, FunctorClass>::template Do<TypesUsage_>(ParamsUsage_); \
		}

		DETAIL_TOOLKIT_DECLARE_FOREACHINTYPELIST_DO(MK_PARAM(TY T1), MK_PARAM(T1), MK_PARAM(const T1& p1), MK_PARAM(p1));
		DETAIL_TOOLKIT_DECLARE_FOREACHINTYPELIST_DO(MK_PARAM(TY T1, TY T2), MK_PARAM(T1, T2), MK_PARAM(const T1& p1, const T2& p2), MK_PARAM(p1, p2));
		DETAIL_TOOLKIT_DECLARE_FOREACHINTYPELIST_DO(MK_PARAM(TY T1, TY T2, TY T3), MK_PARAM(T1, T2, T3), MK_PARAM(const T1& p1, const T2& p2, const T3& p3), MK_PARAM(p1, p2, p3));
		DETAIL_TOOLKIT_DECLARE_FOREACHINTYPELIST_DO(MK_PARAM(TY T1, TY T2, TY T3, TY T4), MK_PARAM(T1, T2, T3, T4), MK_PARAM(const T1& p1, const T2& p2, const T3& p3, const T4& p4), MK_PARAM(p1, p2, p3, p4));
		DETAIL_TOOLKIT_DECLARE_FOREACHINTYPELIST_DO(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5), MK_PARAM(T1, T2, T3, T4, T5), MK_PARAM(const T1& p1, const T2& p2, const T3& p3, const T4& p4, const T5& p5), MK_PARAM(p1, p2, p3, p4, p5));

#undef DETAIL_TOOLKIT_DECLARE_FOREACHINTYPELIST_DO
	};

	template < template <typename> class FunctorClass >
	struct ForEachInTypeList<TypeListEndNode, FunctorClass>
	{
		static void Do() { }

		template <TY T1> static void Do(const T1&) { }
		template <TY T1, TY T2> static void Do(const T1&, const T2&) { }
		template <TY T1, TY T2, TY T3> static void Do(const T1&, const T2&, const T3&) { }
		template <TY T1, TY T2, TY T3, TY T4> static void Do(const T1&, const T2&, const T3&, const T4&) { }
		template <TY T1, TY T2, TY T3, TY T4, TY T5> static void Do(const T1&, const T2&, const T3&, const T4&, const T5&) { }
	};



	namespace Detail
	{

		template < typename T, bool IsTypeList >
		struct ToTypeListImpl { typedef T	ValueT; };

		template < typename T >
		struct ToTypeListImpl<T, false> { typedef TypeList_1<T>	ValueT; };

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

#undef TY

}


#endif
