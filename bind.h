#ifndef STINGRAY_TOOLKIT_BIND_H
#define STINGRAY_TOOLKIT_BIND_H


#include <stingray/toolkit/function.h>
#include <stingray/toolkit/FunctorInvoker.h>
#include <stingray/toolkit/Macro.h>
#include <stingray/toolkit/MetaProgramming.h>
#include <stingray/toolkit/reference.h>
#include <stingray/toolkit/shared_ptr.h>
#include <stingray/toolkit/Tuple.h>
#include <stingray/toolkit/TypeList.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

#define TY typename

	template < size_t N > struct BindPlaceholder
	{ static const size_t Index = N; };

	static inline BindPlaceholder<0>	_1()	{ return BindPlaceholder<0>(); }
	static inline BindPlaceholder<1>	_2()	{ return BindPlaceholder<1>(); }
	static inline BindPlaceholder<2>	_3()	{ return BindPlaceholder<2>(); }
	static inline BindPlaceholder<3>	_4()	{ return BindPlaceholder<3>(); }
	static inline BindPlaceholder<4>	_5()	{ return BindPlaceholder<4>(); }
	static inline BindPlaceholder<5>	_6()	{ return BindPlaceholder<5>(); }
	static inline BindPlaceholder<6>	_7()	{ return BindPlaceholder<6>(); }
	static inline BindPlaceholder<7>	_8()	{ return BindPlaceholder<7>(); }
	static inline BindPlaceholder<8>	_9()	{ return BindPlaceholder<8>(); }
	static inline BindPlaceholder<9>	_10()	{ return BindPlaceholder<9>(); }

	namespace Detail
	{
		template < size_t N > struct Chomper
		{ static const size_t Index = N; };

		template < typename T > struct IsPlaceholder							{ static const bool Value = false; };
		template < size_t N > struct IsPlaceholder<BindPlaceholder<N>(*)() >	{ static const bool Value = true; };
		template < size_t N > struct IsPlaceholder<Chomper<N> >					{ static const bool Value = true; };


		template < typename T >
		struct GetPlaceholderIndex
		{ static const int Value = -1; };

		template < size_t N >
		struct GetPlaceholderIndex<BindPlaceholder<N>(*)() >
		{ static const int Value = N; };

		template < size_t N >
		struct GetPlaceholderIndex<Chomper<N> >
		{ static const int Value = N; };

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
			template<typename T>
			AbsentParamDummy(const T&) {}
		};

		template < typename OriginalParamTypes, typename AllParameters, size_t CurrentIndex, bool HasThisParam = IndexOfTypeListItem<AllParameters, BindPlaceholder<CurrentIndex>(*)() >::Value != -1 >
		struct BinderSingleParamTypeGetter
		{ typedef AbsentParamDummy ValueT; };

		template < typename OriginalParamTypes, typename AllParameters, size_t CurrentIndex >
		struct BinderSingleParamTypeGetter<OriginalParamTypes, AllParameters, CurrentIndex, true>
		{ typedef typename GetTypeListItem<OriginalParamTypes, IndexOfTypeListItem<AllParameters, BindPlaceholder<CurrentIndex>(*)() >::Value >::ValueT	ValueT; };

		template < typename OriginalParamTypes, typename AllParameters, size_t CurrentIndex = 0, size_t Count = GetBinderParamsCount<AllParameters>::Value >
		struct BinderParamTypesGetter
		{ typedef TypeListNode<typename BinderSingleParamTypeGetter<OriginalParamTypes, AllParameters, CurrentIndex>::ValueT, typename BinderParamTypesGetter<OriginalParamTypes, AllParameters, CurrentIndex + 1, Count>::ValueT> ValueT; };

		template < typename OriginalParamTypes, typename AllParameters, size_t Count >
		struct BinderParamTypesGetter<OriginalParamTypes, AllParameters, Count, Count>
		{ typedef TypeListEndNode	ValueT; };

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
			typedef TypeListNode<IntToType<Num>, typename BoundParamNumbersGetter<typename AllParameters::Next, SrcAllParameters>::ValueT>	ValueT;
		};

		template < typename SrcAllParameters >
		struct BoundParamNumbersGetter<TypeListEndNode, SrcAllParameters>
		{
			typedef TypeListEndNode ValueT;
			static const int Counter = GetTypeListLength<typename TypeListCopyIf<SrcAllParameters, Not<IsPlaceholder>::template ValueT>::ValueT>::Value;
		};

		template < typename AllParameters >
		struct GetBinderParameters : public TypeListCopyIf<AllParameters, IsPlaceholder> { };

		template < typename OriginalParamType, typename BinderParams >
		struct GetParamType
		{ typedef typename GetParamPassingType<OriginalParamType>::ValueT ValueT; };

		template < size_t Index, typename BinderParams >
		struct GetParamType<BindPlaceholder<Index>(*)(), BinderParams>
		{ typedef typename GetParamPassingType<typename GetTypeListItem<BinderParams, Index>::ValueT>::ValueT	ValueT; };


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
				return GetTupleItem<GetPlaceholderIndex<Placeholder>::Value>(binderParams);
			}
		};

		template < typename AllParameters, typename BinderParams, size_t Index >
		struct ParamSelector<AllParameters, BinderParams, Index, false>
		{
			typedef typename BoundParamTypesGetter<AllParameters>::ValueT	BoundParams;
			static typename GetParamType<typename GetTypeListItem<AllParameters, Index>::ValueT, BinderParams>::ValueT
			Get(const Tuple<BoundParams>& boundParams, const Tuple<BinderParams>& binderParams)
			{ return GetTupleItem<GetTypeListItem<typename BoundParamNumbersGetter<AllParameters>::ValueT, Index>::ValueT::Value>(boundParams); }
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
			typename GetParamPassingType<typename GetTypeListItem<TypeList, Index>::ValueT>::ValueT Get() const
			{ return _allParams.template Get<IndexOfTypeListItem<BoundParamNumbers, IntToType<Index> >::Value>(); }
		};

		template<typename T>
		struct BindCallParamTraits
		{
			typedef typename GetParamPassingType<T>::ValueT ValueT;
		};

		template<typename T>
		struct ToPointerCallProxy
		{
		private:
			T*				_rawPtr;

		public:
			ToPointerCallProxy(T* ptr)
				: _rawPtr(ptr)
			{}
			ToPointerCallProxy(T& ref)
				: _rawPtr(&ref)
			{}
			template < typename U >
			ToPointerCallProxy(const shared_ptr<U>& ptr)
				: _rawPtr(ptr.get())
			{}
			operator T*() const { return _rawPtr; }
		};

		template< >
		struct ToPointerCallProxy<void>
		{
		private:
			void* _rawPtr;
		public:
			ToPointerCallProxy(void* ptr)
				: _rawPtr(ptr)
			{}
			operator void*() const { return _rawPtr; }
		};

		template< >
		struct ToPointerCallProxy<const void>
		{
		private:
			const void* _rawPtr;

		public:
			ToPointerCallProxy(const void* ptr)
				: _rawPtr(ptr)
			{}
			operator const void*() const { return _rawPtr; }
		};

		template<typename T>
		struct BindCallParamTraits<T*>
		{
			typedef const ToPointerCallProxy<T>& ValueT;
		};

		template<typename T>
		struct IsNotChomped
		{ static const bool Value = true; };

		template<size_t N>
		struct IsNotChomped<Chomper<N> >
		{ static const bool Value = false; };

		template < typename RetType_, typename AllParameters, typename FunctorType >
		class BinderBase
		{
		public:
			typedef RetType_																									RetType;
			typedef typename BinderParamTypesGetter<typename function_info<FunctorType>::ParamTypes, AllParameters>::ValueT		ParamTypes;

		protected:
			typedef typename BoundParamTypesGetter<AllParameters>::ValueT	BoundParams;

			template < typename BinderParams >
			class RealParameters
			{
				typedef typename TypeListCopyIf<AllParameters, IsNotChomped>::ValueT RealRealParameters;
			public:
				static const size_t Size = GetTypeListLength<RealRealParameters>::Value;

			private:
				const Tuple<BoundParams>&		_boundParams;
				const Tuple<BinderParams>&		_binderParams;

			public:
				RealParameters(const Tuple<BoundParams>& boundParams, const Tuple<BinderParams>& binderParams)
					: _boundParams(boundParams), _binderParams(binderParams)
				{ }

				template < size_t Index >
				inline typename GetParamType<typename GetTypeListItem<RealRealParameters, Index>::ValueT, BinderParams>::ValueT
				Get() const
				{ return ParamSelector<AllParameters, BinderParams, Index>::Get(_boundParams, _binderParams); }
			};

		protected:
			FunctorType					_func;
			Tuple<BoundParams>			_boundParams;

		protected:
			BinderBase(const FunctorType& func, const Tuple<AllParameters>& allParams)
				: _func(func), _boundParams(GetBoundParams(allParams))
			{ }

		private:
			static inline Tuple<BoundParams> GetBoundParams(const Tuple<AllParameters>& all)
			{ return Tuple<BoundParams>::CreateFromTupleLikeObject(NonPlaceholdersCutter<AllParameters>(all)); }
		};


		template
			<
				typename RetType,
				typename AllParameters,
				typename FunctorType,
				size_t BinderParamsCount = GetBinderParamsCount<AllParameters>::Value
			>
		struct Binder;

		template < typename RetType_, typename AllParameters, typename FunctorType >
		struct Binder<RetType_, AllParameters, FunctorType, 0> : private BinderBase<RetType_, AllParameters, FunctorType>, public function_info<RetType_, typename BinderParamTypesGetter<typename function_info<FunctorType>::ParamTypes, AllParameters>::ValueT>
		{
			typedef BinderBase<RetType_, AllParameters, FunctorType>	base;
			typedef typename base::RetType								RetType;
			typedef typename base::ParamTypes							ParamTypes;

			Binder(const FunctorType& func, const Tuple<AllParameters>& allParams)
				: base(func, allParams)
			{ }

			inline RetType operator () () const
			{
				typedef TypeList_0	BinderParams;
				Tuple<BinderParams> call_params;
				typename base::template RealParameters<BinderParams> rp(base::_boundParams, call_params);
				return FunctorInvoker::Invoke<FunctorType>(base::_func, rp);
			}

			std::string GetFuncName() const { return get_function_name(base::_func); }
		};

#define DETAIL_TOOLKIT_DECLARE_BINDER(N, CallParamsDecl_, CallParamsUsage_) \
		template < typename RetType_, typename AllParameters, typename FunctorType > \
		struct Binder<RetType_, AllParameters, FunctorType, N> : private BinderBase<RetType_, AllParameters, FunctorType>, public function_info<RetType_, typename BinderParamTypesGetter<typename function_info<FunctorType>::ParamTypes, AllParameters>::ValueT> \
		{ \
			typedef BinderBase<RetType_, AllParameters, FunctorType>					base; \
			typedef typename base::RetType												RetType; \
			typedef typename base::ParamTypes											ParamTypes; \
			typedef typename TypeListTransform<ParamTypes, BindCallParamTraits>::ValueT	CallProxiesParamTypes; \
			\
			Binder(const FunctorType& func, const Tuple<AllParameters>& allParams) \
				: base(func, allParams) \
			{ } \
			\
			inline RetType operator () (CallParamsDecl_) const  \
			{ \
				Tuple<ParamTypes> call_params(CallParamsUsage_); \
				typename base::template RealParameters<ParamTypes> rp(base::_boundParams, call_params); \
				return FunctorInvoker::Invoke<FunctorType>(base::_func, rp); \
			} \
			\
			std::string GetFuncName() const { return get_function_name(base::_func); } \
		}

#define P_(N) typename GetTypeListItem<CallProxiesParamTypes, N - 1>::ValueT p##N

		DETAIL_TOOLKIT_DECLARE_BINDER(1, MK_PARAM(P_(1)), MK_PARAM(p1));
		DETAIL_TOOLKIT_DECLARE_BINDER(2, MK_PARAM(P_(1), P_(2)), MK_PARAM(p1, p2));
		DETAIL_TOOLKIT_DECLARE_BINDER(3, MK_PARAM(P_(1), P_(2), P_(3)), MK_PARAM(p1, p2, p3));
		DETAIL_TOOLKIT_DECLARE_BINDER(4, MK_PARAM(P_(1), P_(2), P_(3), P_(4)), MK_PARAM(p1, p2, p3, p4));
		DETAIL_TOOLKIT_DECLARE_BINDER(5, MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5)), MK_PARAM(p1, p2, p3, p4, p5));
		DETAIL_TOOLKIT_DECLARE_BINDER(6, MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6)), MK_PARAM(p1, p2, p3, p4, p5, p6));
		DETAIL_TOOLKIT_DECLARE_BINDER(7, MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7));
		DETAIL_TOOLKIT_DECLARE_BINDER(8, MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8));
		DETAIL_TOOLKIT_DECLARE_BINDER(9, MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9));
		DETAIL_TOOLKIT_DECLARE_BINDER(10, MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10));

#undef P_

	}

#define DETAIL_TOOLKIT_DECLARE_BIND(TemplateBindParams_, BindParamTypes_, BindParamsDecl_, BindParamsUsage_) \
	template < typename FunctorType, TemplateBindParams_ > \
	Detail::Binder<typename function_info<FunctorType>::RetType, TYPELIST(BindParamTypes_), FunctorType > \
		bind(const FunctorType& func, BindParamsDecl_) \
	{ \
		typedef typename function_info<FunctorType>::RetType RetType; \
		typedef TYPELIST(BindParamTypes_)	AllParams; \
		Tuple<AllParams> all_params(BindParamsUsage_); \
		return Detail::Binder<RetType, AllParams, FunctorType>(func, all_params); \
	}

	DETAIL_TOOLKIT_DECLARE_BIND(MK_PARAM(TY T1), MK_PARAM(T1), MK_PARAM(T1 p1), MK_PARAM(p1))
	DETAIL_TOOLKIT_DECLARE_BIND(MK_PARAM(TY T1, TY T2), MK_PARAM(T1, T2), MK_PARAM(T1 p1, T2 p2), MK_PARAM(p1, p2))
	DETAIL_TOOLKIT_DECLARE_BIND(MK_PARAM(TY T1, TY T2, TY T3), MK_PARAM(T1, T2, T3), MK_PARAM(T1 p1, T2 p2, T3 p3), MK_PARAM(p1, p2, p3))
	DETAIL_TOOLKIT_DECLARE_BIND(MK_PARAM(TY T1, TY T2, TY T3, TY T4), MK_PARAM(T1, T2, T3, T4), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4), MK_PARAM(p1, p2, p3, p4))
	DETAIL_TOOLKIT_DECLARE_BIND(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5), MK_PARAM(T1, T2, T3, T4, T5), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5), MK_PARAM(p1, p2, p3, p4, p5))
	DETAIL_TOOLKIT_DECLARE_BIND(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6), MK_PARAM(T1, T2, T3, T4, T5, T6), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6), MK_PARAM(p1, p2, p3, p4, p5, p6))
	DETAIL_TOOLKIT_DECLARE_BIND(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7), MK_PARAM(T1, T2, T3, T4, T5, T6, T7), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7), MK_PARAM(p1, p2, p3, p4, p5, p6, p7))
	DETAIL_TOOLKIT_DECLARE_BIND(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8))
	DETAIL_TOOLKIT_DECLARE_BIND(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8, T9 p9), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9))
	DETAIL_TOOLKIT_DECLARE_BIND(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8, T9 p9, T10 p10), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10))

#undef TY

	template<size_t N>
	Detail::Chomper<N> not_using(BindPlaceholder<N>(*)()) { return Detail::Chomper<N>(); }

	/** @} */

}


#endif
