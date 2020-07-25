#ifndef STINGRAYKIT_FUNCTION_BIND_H
#define STINGRAYKIT_FUNCTION_BIND_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/function/FunctorInvoker.h>
#include <stingraykit/function/function_name_getter.h>
#include <stingraykit/PerfectForwarding.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

	namespace Detail
	{

		template < size_t N > struct Placeholder
		{ static const size_t Index = N; };

		template < size_t N > struct Chomper
		{ static const size_t Index = N; };

		template < typename T > struct IsPlaceholderImpl						: FalseType { };
		template < size_t N > struct IsPlaceholderImpl<Placeholder<N> >			: TrueType { };
		template < size_t N > struct IsPlaceholderImpl<Chomper<N> >				: TrueType { };

		template < typename T > struct IsPlaceholder							: IsPlaceholderImpl<typename Decay<T>::ValueT> { };

		template < typename T > struct IsChomperImpl							: FalseType { };
		template < size_t N > struct IsChomperImpl<Chomper<N> >					: TrueType { };

		template < typename T > struct IsChomper								: IsChomperImpl<typename Decay<T>::ValueT> { };

		template < typename T > struct GetPlaceholderIndexImpl						: integral_constant<int, -1> { };
		template < size_t N > struct GetPlaceholderIndexImpl<Placeholder<N> >		: integral_constant<int, N> { };
		template < size_t N > struct GetPlaceholderIndexImpl<Chomper<N> >			: integral_constant<int, N> { };

		template < typename T > struct GetPlaceholderIndex							: GetPlaceholderIndexImpl<typename Decay<T>::ValueT> { };

		template < typename AllParameters >
		struct GetBinderParamsCount
		{
			static const size_t Value =
					(GetBinderParamsCount<typename AllParameters::Next>::Value > GetPlaceholderIndex<typename AllParameters::ValueT>::Value + 1) ?
					GetBinderParamsCount<typename AllParameters::Next>::Value :
					(GetPlaceholderIndex<typename AllParameters::ValueT>::Value + 1);
		};

		template < >
		struct GetBinderParamsCount<TypeListEndNode>
		{ static const size_t Value = 0; };


		struct AbsentParamDummy
		{
			AbsentParamDummy() { }

			template < typename T >
			AbsentParamDummy(const T&) { }
		};

		template < typename OriginalParamTypes, typename AllParameters, size_t CurrentIndex, bool HasThisParam = IndexOfTypeListItem<AllParameters, Placeholder<CurrentIndex> >::Value != TypeListNpos >
		struct BinderSingleParamTypeGetter
		{ typedef AbsentParamDummy ValueT; };

		template < typename OriginalParamTypes, typename AllParameters, size_t CurrentIndex >
		struct BinderSingleParamTypeGetter<OriginalParamTypes, AllParameters, CurrentIndex, true>
		{ typedef typename GetTypeListItem<OriginalParamTypes, IndexOfTypeListItem<AllParameters, Placeholder<CurrentIndex> >::Value >::ValueT ValueT; };

		template < typename OriginalParamTypes, typename AllParameters, size_t CurrentIndex = 0, size_t Count = GetBinderParamsCount<AllParameters>::Value >
		struct BinderParamTypesGetter
		{ typedef TypeListNode<typename BinderSingleParamTypeGetter<OriginalParamTypes, AllParameters, CurrentIndex>::ValueT, typename BinderParamTypesGetter<OriginalParamTypes, AllParameters, CurrentIndex + 1, Count>::ValueT> ValueT; };

		template < typename OriginalParamTypes, typename AllParameters, size_t Count >
		struct BinderParamTypesGetter<OriginalParamTypes, AllParameters, Count, Count>
		{ typedef TypeListEndNode ValueT; };

		template < typename AllParameters, size_t CurrentIndex, size_t Count >
		struct BinderParamTypesGetter<UnspecifiedParamTypes, AllParameters, CurrentIndex, Count>
		{ typedef UnspecifiedParamTypes ValueT; };

		template < typename AllParameters, size_t Count >
		struct BinderParamTypesGetter<UnspecifiedParamTypes, AllParameters, Count, Count>
		{ typedef UnspecifiedParamTypes ValueT; };

		template < typename AllParameters >
		struct BoundParamTypesGetter
		{ typedef typename TypeListCopyIf<AllParameters, Not<IsPlaceholder>::template ValueT>::ValueT ValueT; };


		template < typename AllParameters, typename SrcAllParameters = AllParameters >
		struct BoundParamNumbersGetter
		{
			typedef typename AllParameters::ValueT CurType;
			static const bool IsPH = IsPlaceholder<CurType>::Value;
			static const int Counter = BoundParamNumbersGetter<typename AllParameters::Next, SrcAllParameters>::Counter - (IsPH ? 0 : 1);
			static const int Num = IsPH ? -1 : Counter;
			typedef TypeListNode<IntToType<Num>, typename BoundParamNumbersGetter<typename AllParameters::Next, SrcAllParameters>::ValueT> ValueT;
		};

		template < typename SrcAllParameters >
		struct BoundParamNumbersGetter<TypeListEndNode, SrcAllParameters>
		{
			typedef TypeListEndNode ValueT;
			static const int Counter = GetTypeListLength<typename TypeListCopyIf<SrcAllParameters, Not<IsPlaceholder>::template ValueT>::ValueT>::Value;
		};

		template < typename OriginalParamType, typename BinderParams >
		struct GetParamType
		{ typedef const OriginalParamType& ValueT; };

		template < size_t Index, typename BinderParams >
		struct GetParamType<Placeholder<Index>, BinderParams>
		{ typedef typename GetTypeListItem<BinderParams, Index>::ValueT ValueT; };


		template
			<
				typename AllParameters,
				typename BinderParams,
				size_t Index,
				bool UseBinderParams = IsPlaceholder<typename GetTypeListItem<AllParameters, Index>::ValueT>::Value
			>
		struct ParamSelector;

		template < typename AllParameters, typename BinderParams, size_t Index >
		struct ParamSelector<AllParameters, BinderParams, Index, true>
		{
			typedef typename BoundParamTypesGetter<AllParameters>::ValueT	BoundParams;

			static typename GetParamType<typename GetTypeListItem<AllParameters, Index>::ValueT, BinderParams>::ValueT
			Get(const Tuple<BoundParams>& BoundParams, const Tuple<BinderParams>& binderParams)
			{
				typedef typename GetTypeListItem<AllParameters, Index>::ValueT Placeholder;
				return binderParams.template Get<GetPlaceholderIndex<Placeholder>::Value>();
			}
		};

		template < typename AllParameters, typename BinderParams, size_t Index >
		struct ParamSelector<AllParameters, BinderParams, Index, false>
		{
			typedef typename BoundParamTypesGetter<AllParameters>::ValueT	BoundParams;

			static typename GetParamType<typename GetTypeListItem<AllParameters, Index>::ValueT, BinderParams>::ValueT
			Get(const Tuple<BoundParams>& boundParams, const Tuple<BinderParams>& binderParams)
			{ return boundParams.template Get<GetTypeListItem<typename BoundParamNumbersGetter<AllParameters>::ValueT, Index>::ValueT::Value>(); }
		};


		template < typename AllParameters >
		class NonPlaceholdersCutter
		{
		public:
			typedef typename BoundParamTypesGetter<AllParameters>::ValueT		TypeList;
			typedef typename BoundParamNumbersGetter<AllParameters>::ValueT	BoundParamNumbers;

		private:
			const Tuple<AllParameters>&		_allParams;

		public:
			NonPlaceholdersCutter(const Tuple<AllParameters>& allParams) : _allParams(allParams) { }

			template < size_t Index >
			const typename GetTypeListItem<TypeList, Index>::ValueT& Get() const
			{ return _allParams.template Get<IndexOfTypeListItem<BoundParamNumbers, IntToType<Index> >::Value>(); }
		};

		template < typename FunctorType, typename AllParameters >
		class Binder
		{
			typedef typename BoundParamTypesGetter<AllParameters>::ValueT	BoundParams;

		public:
			typedef typename function_info<FunctorType>::RetType															RetType;
			typedef typename BinderParamTypesGetter<typename function_info<FunctorType>::ParamTypes, AllParameters>::ValueT	ParamTypes;

		private:
			template < typename BinderParams >
			class RealParameters
			{
			public:
				typedef typename TypeListCopyIf<AllParameters, Not<IsChomper>::template ValueT>::ValueT TypeList;

				static const size_t Size = GetTypeListLength<TypeList>::Value;

			private:
				const Tuple<BoundParams>&		_boundParams;
				const Tuple<BinderParams>&		_binderParams;

			public:
				RealParameters(const Tuple<BoundParams>& boundParams, const Tuple<BinderParams>& binderParams)
					: _boundParams(boundParams), _binderParams(binderParams)
				{ }

				template < size_t Index >
				typename GetParamType<typename GetTypeListItem<TypeList, Index>::ValueT, BinderParams>::ValueT Get() const
				{ return ParamSelector<AllParameters, BinderParams, Index>::Get(_boundParams, _binderParams); }
			};

		private:
			FunctorType					_func;
			Tuple<BoundParams>			_boundParams;

		public:
			Binder(const FunctorType& func, const Tuple<AllParameters>& allParams)
				: _func(func), _boundParams(TupleConstructorTag(), NonPlaceholdersCutter<AllParameters>(allParams))
			{ }

			STINGRAYKIT_PERFECT_FORWARDING(RetType, operator(), Do)

			std::string get_name() const { return "{ binder: " + get_function_name(_func) + " }"; }

		private:
			template < typename ParamTypeList >
			RetType Do(const Tuple<ParamTypeList>& params) const
			{
				RealParameters<ParamTypeList> rp(_boundParams, params);
				return FunctorInvoker::Invoke(_func, rp);
			}
		};

	}


#define TY typename
#define DETAIL_STINGRAYKIT_DECLARE_BIND(TemplateBindParams_, BindParamTypes_, BindParamsDecl_, BindParamsUsage_) \
	template < typename FunctorType, TemplateBindParams_ > \
	Detail::Binder<FunctorType, typename TypeList<BindParamTypes_>::type> \
		Bind(const FunctorType& func, BindParamsDecl_) \
	{ \
		typedef typename TypeList<BindParamTypes_>::type		AllParameters; \
		Tuple<AllParameters> allParams(BindParamsUsage_); \
		return Detail::Binder<FunctorType, AllParameters>(func, allParams); \
	}

	DETAIL_STINGRAYKIT_DECLARE_BIND(MK_PARAM(TY T1), MK_PARAM(T1), MK_PARAM(T1 p1), MK_PARAM(p1))
	DETAIL_STINGRAYKIT_DECLARE_BIND(MK_PARAM(TY T1, TY T2), MK_PARAM(T1, T2), MK_PARAM(T1 p1, T2 p2), MK_PARAM(p1, p2))
	DETAIL_STINGRAYKIT_DECLARE_BIND(MK_PARAM(TY T1, TY T2, TY T3), MK_PARAM(T1, T2, T3), MK_PARAM(T1 p1, T2 p2, T3 p3), MK_PARAM(p1, p2, p3))
	DETAIL_STINGRAYKIT_DECLARE_BIND(MK_PARAM(TY T1, TY T2, TY T3, TY T4), MK_PARAM(T1, T2, T3, T4), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4), MK_PARAM(p1, p2, p3, p4))
	DETAIL_STINGRAYKIT_DECLARE_BIND(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5), MK_PARAM(T1, T2, T3, T4, T5), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5), MK_PARAM(p1, p2, p3, p4, p5))
	DETAIL_STINGRAYKIT_DECLARE_BIND(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6), MK_PARAM(T1, T2, T3, T4, T5, T6), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6), MK_PARAM(p1, p2, p3, p4, p5, p6))
	DETAIL_STINGRAYKIT_DECLARE_BIND(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7), MK_PARAM(T1, T2, T3, T4, T5, T6, T7), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7), MK_PARAM(p1, p2, p3, p4, p5, p6, p7))
	DETAIL_STINGRAYKIT_DECLARE_BIND(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8))
	DETAIL_STINGRAYKIT_DECLARE_BIND(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8, T9 p9), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9))
	DETAIL_STINGRAYKIT_DECLARE_BIND(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8, T9 p9, T10 p10), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10))

#undef DETAIL_STINGRAYKIT_DECLARE_BIND
#undef TY


#define DETAIL_STINGRAYKIT_DECLARE_PLACEHOLDER(Index_, UserArg_) \
	extern const Detail::Placeholder<Index_>	STINGRAYKIT_CAT(_, STINGRAYKIT_INC(Index_));

	STINGRAYKIT_REPEAT(20, DETAIL_STINGRAYKIT_DECLARE_PLACEHOLDER, ~)

#undef DETAIL_STINGRAYKIT_DECLARE_PLACEHOLDER


	template < size_t N >
	Detail::Chomper<N> not_using(const Detail::Placeholder<N>&) { return Detail::Chomper<N>(); }

	/** @} */

}

#endif
