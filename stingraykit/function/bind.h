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

namespace stingray
{

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

	namespace Detail
	{

		template < typename FunctorType, typename... Ts >
		class Binder;

		template < size_t N > struct Placeholder
		{ static const size_t Index = N; };

		template < size_t N > struct Chomper
		{ static const size_t Index = N; };

		template < typename T > struct IsBinderImpl								: FalseType { };
		template < typename FunctorType, typename... Ts >
		struct IsBinderImpl<Binder<FunctorType, Ts...> >						: TrueType { };

		template < typename T > struct IsBinder									: IsBinderImpl<typename Decay<T>::ValueT> { };

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

		template < typename OriginalParamType, typename BinderParams, bool ForwardBinderParams = false >
		struct GetParamType
		{ typedef const OriginalParamType& ValueT; };

		template < typename FunctorType, typename... Ts, typename BinderParams, bool ForwardBinderParams >
		struct GetParamType<Binder<FunctorType, Ts...>, BinderParams, ForwardBinderParams>
		{ typedef typename Binder<FunctorType, Ts...>::RetType ValueT; };

		template < size_t Index, typename BinderParams >
		struct GetParamType<Placeholder<Index>, BinderParams, true>
		{ typedef typename GetTypeListItem<BinderParams, Index>::ValueT ValueT; };

		template < size_t Index, typename BinderParams >
		struct GetParamType<Placeholder<Index>, BinderParams, false>
		{ typedef const typename GetTypeListItem<BinderParams, Index>::ValueT& ValueT; };


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
			typedef typename GetTypeListItem<AllParameters, Index>::ValueT	BoundType;
			typedef typename GetParamType<BoundType, BinderParams>::ValueT	ParamType;
			typedef typename BoundParamTypesGetter<AllParameters>::ValueT	BoundParams;

			static ParamType Get(const Tuple<BoundParams>& BoundParams, const Tuple<BinderParams>& binderParams)
			{ return binderParams.template Get<GetPlaceholderIndex<BoundType>::Value>(); }

			static typename GetParamType<BoundType, BinderParams, true>::ValueT Get(const Tuple<BoundParams>& BoundParams, Tuple<BinderParams>&& binderParams)
			{ return std::move(binderParams).template Get<GetPlaceholderIndex<BoundType>::Value>(); }
		};

		template < typename AllParameters, typename BinderParams, size_t Index >
		struct ParamSelector<AllParameters, BinderParams, Index, false>
		{
			typedef typename GetTypeListItem<AllParameters, Index>::ValueT	BoundType;
			typedef typename GetParamType<BoundType, BinderParams>::ValueT	ParamType;
			typedef typename BoundParamTypesGetter<AllParameters>::ValueT	BoundParams;

			static ParamType Get(const Tuple<BoundParams>& boundParams, const Tuple<BinderParams>& binderParams)
			{ return DoGet<BoundType>(boundParams, binderParams); }

			static ParamType Get(const Tuple<BoundParams>& boundParams, Tuple<BinderParams>&& binderParams)
			{
				CompileTimeAssert<IsSame<ParamType, typename GetParamType<BoundType, BinderParams, true>::ValueT>::Value> ERROR__trying_to_forward_bound_params;
				return DoGet<BoundType>(boundParams, std::move(binderParams));
			}

		private:
			template < typename BoundType_, typename BinderParamsTuple_ >
			static ParamType DoGet(const Tuple<BoundParams>& boundParams, BinderParamsTuple_&& binderParams, typename EnableIf<!IsBinder<BoundType_>::Value, Dummy>::ValueT* = 0)
			{ return boundParams.template Get<GetTypeListItem<typename BoundParamNumbersGetter<AllParameters>::ValueT, Index>::ValueT::Value>(); }

			template < typename BoundType_, typename BinderParamsTuple_ >
			static ParamType DoGet(const Tuple<BoundParams>& boundParams, BinderParamsTuple_&& binderParams, typename EnableIf<IsBinder<BoundType_>::Value, Dummy>::ValueT* = 0)
			{ return FunctorInvoker::Invoke(boundParams.template Get<GetTypeListItem<typename BoundParamNumbersGetter<AllParameters>::ValueT, Index>::ValueT::Value>(), std::forward<BinderParamsTuple_>(binderParams)); }
		};


		template < typename... Ts >
		class NonPlaceholdersCutter
		{
		public:
			typedef TypeList<Ts&&...>										AllParameters;
			typedef typename BoundParamTypesGetter<AllParameters>::ValueT	Types;
			typedef typename BoundParamNumbersGetter<AllParameters>::ValueT	BoundParamNumbers;

		private:
			Tuple<AllParameters>		_allParams;

		public:
			NonPlaceholdersCutter(Ts&&... args) : _allParams(std::forward<Ts>(args)...) { }

			template < size_t Index >
			const typename GetTypeListItem<Types, Index>::ValueT& Get() const &
			{ return _allParams.template Get<IndexOfTypeListItem<BoundParamNumbers, IntToType<Index> >::Value>(); }

			template < size_t Index >
			typename EnableIf<IsNonConstRvalueReference<typename GetTypeListItem<Types, Index>::ValueT>::Value, typename GetTypeListItem<Types, Index>::ValueT>::ValueT Get() &&
			{ return std::move(_allParams).template Get<IndexOfTypeListItem<BoundParamNumbers, IntToType<Index> >::Value>(); }
		};

		template < typename FunctorType, typename... Ts >
		class Binder
		{
			typedef typename Decay<FunctorType>::ValueT						RawFunctorType;
			typedef TypeList<typename Decay<Ts>::ValueT...>					AllParameters;
			typedef typename BoundParamTypesGetter<AllParameters>::ValueT	BoundParams;

		public:
			typedef typename function_info<RawFunctorType>::RetType																RetType;
			typedef typename BinderParamTypesGetter<typename function_info<RawFunctorType>::ParamTypes, AllParameters>::ValueT	ParamTypes;

		private:
			template < typename... Us >
			class RealParameters
			{
			public:
				typedef TypeList<Us&&...>	BinderParams;

				typedef typename TypeListCopyIf<AllParameters, Not<IsChomper>::template ValueT>::ValueT Types;

				static const size_t Size = GetTypeListLength<Types>::Value;

			private:
				const Tuple<BoundParams>&		_boundParams;
				Tuple<BinderParams>				_binderParams;

			public:
				RealParameters(const Tuple<BoundParams>& boundParams, Us&&... params)
					: _boundParams(boundParams), _binderParams(std::forward<Us>(params)...)
				{ }

				template < size_t Index >
				typename GetParamType<typename GetTypeListItem<Types, Index>::ValueT, BinderParams>::ValueT Get() const &
				{ return ParamSelector<AllParameters, BinderParams, Index>::Get(_boundParams, _binderParams); }

				template < size_t Index >
				typename EnableIf<IsNonConstRvalueReference<typename GetParamType<typename GetTypeListItem<Types, Index>::ValueT, BinderParams, true>::ValueT>::Value ||
								IsBinder<typename GetTypeListItem<Types, Index>::ValueT>::Value,
						typename GetParamType<typename GetTypeListItem<Types, Index>::ValueT, BinderParams, true>::ValueT>::ValueT Get() &&
				{ return ParamSelector<AllParameters, BinderParams, Index>::Get(_boundParams, std::move(_binderParams)); }
			};

		private:
			RawFunctorType				_func;
			Tuple<BoundParams>			_boundParams;

		public:
			Binder(FunctorType&& func, Ts&&... args)
				: _func(std::forward<FunctorType>(func)), _boundParams(TupleConstructorTag(), NonPlaceholdersCutter<Ts...>(std::forward<Ts>(args)...))
			{ }

			template < typename... Us >
			RetType operator () (Us&&... params) const
			{ return FunctorInvoker::Invoke(_func, RealParameters<Us...>(_boundParams, std::forward<Us>(params)...)); }

			std::string get_name() const { return "{ binder: " + get_function_name(_func) + " }"; }
		};

	}


	template < typename FunctorType, typename... Ts >
	Detail::Binder<FunctorType, Ts...> Bind(FunctorType&& func, Ts&&... args)
	{ return Detail::Binder<FunctorType, Ts...>(std::forward<FunctorType>(func), std::forward<Ts>(args)...); }

#define DETAIL_STINGRAYKIT_DECLARE_PLACEHOLDER(Index_, UserArg_) \
	extern const Detail::Placeholder<Index_>	STINGRAYKIT_CAT(_, STINGRAYKIT_INC(Index_));

	STINGRAYKIT_REPEAT(20, DETAIL_STINGRAYKIT_DECLARE_PLACEHOLDER, ~)

#undef DETAIL_STINGRAYKIT_DECLARE_PLACEHOLDER


	template < size_t N >
	Detail::Chomper<N> not_using(const Detail::Placeholder<N>&) { return Detail::Chomper<N>(); }

	/** @} */

}

#endif
