#ifndef STINGRAY_TOOLKIT_TYPELIST_H
#define STINGRAY_TOOLKIT_TYPELIST_H


#include <sys/types.h>

#include <stingray/toolkit/Macro.h>
#include <stingray/toolkit/MetaProgramming.h>


namespace stingray
{

	/*! \cond GS_INTERNAL */

	struct TypeListEndNode
	{ };


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
	struct TypeList_1 : public TypeListNode<T1, TypeList_0> { };

	namespace Detail
	{
		template < typename Signature >
		struct TypeListCreator;

		template < TY T1 >
		struct TypeListCreator<void(T1)> : public TypeList_1<T1>
		{ };
	}

#define TYPELIST(...)	stingray::Detail::TypeListCreator<void ( __VA_ARGS__ ) >

#define DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(Size_, PrevSize_, TypesTypenames_, Tail_) \
	template < TypesTypenames_ > \
	struct TypeList_##Size_ : public TypeListNode<T1, TypeList_##PrevSize_<Tail_> > { }; \
	\
	namespace Detail { \
		template < TypesTypenames_ > \
		struct TypeListCreator<void(T1, Tail_)> : public TypeList_##Size_<T1, Tail_> \
		{ }; \
	}

	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(2, 1, MK_PARAM(TY T1, TY T2), MK_PARAM(T2))
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(3, 2, MK_PARAM(TY T1, TY T2, TY T3), MK_PARAM(T2, T3))
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(4, 3, MK_PARAM(TY T1, TY T2, TY T3, TY T4), MK_PARAM(T2, T3, T4))
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(5, 4, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5), MK_PARAM(T2, T3, T4, T5))
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(6, 5, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6), MK_PARAM(T2, T3, T4, T5, T6))
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(7, 6, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7), MK_PARAM(T2, T3, T4, T5, T6, T7))
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(8, 7, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8), MK_PARAM(T2, T3, T4, T5, T6, T7, T8))
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(9, 8, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9), MK_PARAM(T2, T3, T4, T5, T6, T7, T8, T9))
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(10, 9, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10), MK_PARAM(T2, T3, T4, T5, T6, T7, T8, T9, T10))
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(11, 10, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11), MK_PARAM(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11))
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(12, 11, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12), MK_PARAM(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12))
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(13, 12, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13), MK_PARAM(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13))
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(14, 13, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14), MK_PARAM(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14))
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(15, 14, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15), MK_PARAM(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15))
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(16, 15, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15, TY T16), MK_PARAM(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16))
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(17, 16, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15, TY T16, TY T17), MK_PARAM(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17))
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(18, 17, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15, TY T16, TY T17, TY T18), MK_PARAM(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18))
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(19, 18, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15, TY T16, TY T17, TY T18, TY T19), MK_PARAM(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19))
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(20, 19, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15, TY T16, TY T17, TY T18, TY T19, TY T20), MK_PARAM(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20))
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(21, 20, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15, TY T16, TY T17, TY T18, TY T19, TY T20, TY T21), MK_PARAM(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21))
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(22, 21, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15, TY T16, TY T17, TY T18, TY T19, TY T20, TY T21, TY T22), MK_PARAM(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22))
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(23, 22, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15, TY T16, TY T17, TY T18, TY T19, TY T20, TY T21, TY T22, TY T23), MK_PARAM(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23))
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(24, 23, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15, TY T16, TY T17, TY T18, TY T19, TY T20, TY T21, TY T22, TY T23, TY T24), MK_PARAM(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24))
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(25, 24, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15, TY T16, TY T17, TY T18, TY T19, TY T20, TY T21, TY T22, TY T23, TY T24, TY T25), MK_PARAM(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25))
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(26, 25, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15, TY T16, TY T17, TY T18, TY T19, TY T20, TY T21, TY T22, TY T23, TY T24, TY T25, TY T26), MK_PARAM(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26))
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(27, 26, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15, TY T16, TY T17, TY T18, TY T19, TY T20, TY T21, TY T22, TY T23, TY T24, TY T25, TY T26, TY T27), MK_PARAM(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27))
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(28, 27, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15, TY T16, TY T17, TY T18, TY T19, TY T20, TY T21, TY T22, TY T23, TY T24, TY T25, TY T26, TY T27, TY T28), MK_PARAM(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28))
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(29, 28, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15, TY T16, TY T17, TY T18, TY T19, TY T20, TY T21, TY T22, TY T23, TY T24, TY T25, TY T26, TY T27, TY T28, TY T29), MK_PARAM(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29))
	DETAIL_DETAIL_TOOLKIT_DECLARE_TYPELIST(30, 29, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY T13, TY T14, TY T15, TY T16, TY T17, TY T18, TY T19, TY T20, TY T21, TY T22, TY T23, TY T24, TY T25, TY T26, TY T27, TY T28, TY T29, TY T30), MK_PARAM(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29, T30))

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
		static const int Value = IndexOfTypeListItem<TypeList, T>::Value != -1;
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


	template < typename TypeList, template <typename> class TransformFunc >
	struct TypeListTransform
	{ typedef TypeListNode<typename TransformFunc<typename TypeList::ValueT>::ValueT, typename TypeListTransform<typename TypeList::Next, TransformFunc>::ValueT> ValueT; };

	template < template <typename> class TransformFunc >
	struct TypeListTransform<TypeListEndNode, TransformFunc>
	{ typedef TypeListEndNode ValueT; };


	template < typename TypeList >
	struct TypeListReverse
	{ typedef typename TypeListAppend<typename TypeListReverse<typename TypeList::Next>::ValueT, typename TypeList::ValueT>::ValueT ValueT;};

	template < >
	struct TypeListReverse<TypeListEndNode>
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
	/*! \endcond */

}


#endif
