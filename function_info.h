#ifndef STINGRAY_TOOLKIT_FUNCTION_INFO_H
#define STINGRAY_TOOLKIT_FUNCTION_INFO_H


#include <stingray/toolkit/Macro.h>
#include <stingray/toolkit/toolkit.h>
#include <stingray/toolkit/TypeList.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

	struct FunctionType
	{
		TOOLKIT_ENUM_VALUES
		(
			RawFunction,
			RawFunctionPtr,
			MethodPtr,
			GSFunction,
			StdFunction,
			Other
		);

		TOOLKIT_DECLARE_ENUM_CLASS(FunctionType);
	};


#ifndef DOXYGEN_PREPROCESSOR

#define TY typename

	template < typename FuncOrRetType, typename OptionalParamTypes = NullType >
	struct function_info;


	template < typename FuncOrRetType, typename OptionalParamTypes = NullType >
	struct function_type;


	struct UnspecifiedParamTypes;


///////////////////////////////////
// for raw functions and methods //
///////////////////////////////////

	template < typename R > struct function_type<R(), NullType>
	{ static const FunctionType::Enum Type = FunctionType::RawFunction; };
	template < typename R > struct function_type<R(*)(), NullType>
	{ static const FunctionType::Enum Type = FunctionType::RawFunctionPtr; };
	template < typename C, typename R > struct function_type<R (C::*)(), NullType>
	{ static const FunctionType::Enum Type = FunctionType::MethodPtr; };
	template < typename C, typename R > struct function_type<R (C::*)() const, NullType>
	{ static const FunctionType::Enum Type = FunctionType::MethodPtr; };
	template < typename R >
	struct function_info<R(), NullType> : public function_type<R()>
	{
		typedef R			RetType;
		typedef TypeList_0	ParamTypes;
		typedef R Signature();
	};
	template < typename R >
	struct function_info<R(*)(), NullType> : public function_type<R(*)()>
	{
		typedef R			RetType;
		typedef TypeList_0	ParamTypes;
		typedef R Signature();
	};
	template < typename C, typename R >
	struct function_info<R (C::*)(), NullType> : public function_type<R (C::*)()>, public std::unary_function<C*, R>
	{
		typedef R				RetType;
		typedef typename TypeList<C*>::type	ParamTypes;
		typedef R Signature(C*);
	};
	template < typename C, typename R >
	struct function_info<R (C::*)() const, NullType> : public function_type<R (C::*)() const>, public std::unary_function<const C*, R>
	{
		typedef R						RetType;
		typedef typename TypeList<const C*>::type	ParamTypes;
		typedef R Signature(const C*);
	};


	template < typename R, typename T1 > struct function_type<R(T1), NullType>
	{ static const FunctionType::Enum Type = FunctionType::RawFunction; };
	template < typename R, typename T1 > struct function_type<R(*)(T1), NullType>
	{ static const FunctionType::Enum Type = FunctionType::RawFunctionPtr; };
	template < typename C, typename R, typename T1 > struct function_type<R (C::*)(T1), NullType>
	{ static const FunctionType::Enum Type = FunctionType::MethodPtr; };
	template < typename C, typename R, typename T1 > struct function_type<R (C::*)(T1) const, NullType>
	{ static const FunctionType::Enum Type = FunctionType::MethodPtr; };
	template < typename R, typename T1 >
	struct function_info<R(T1), NullType> : function_type<R(T1)>, public std::unary_function<T1, R>
	{
		typedef R				RetType;
		typedef typename TypeList<T1>::type	ParamTypes;
		typedef R Signature(T1);
	};
	template < typename R, typename T1 >
	struct function_info<R(*)(T1), NullType> : function_type<R(*)(T1)>, public std::unary_function<T1, R>
	{
		typedef R				RetType;
		typedef typename TypeList<T1>::type	ParamTypes;
		typedef R Signature(T1);
	};
	template < typename C, typename R, typename T1 >
	struct function_info<R (C::*)(T1), NullType> : function_type<R (C::*)(T1)>, public std::binary_function<C*, T1, R>
	{
		typedef R					RetType;
		typedef typename TypeList<C*, T1>::type	ParamTypes;
		typedef R Signature(C*, T1);
	};
	template < typename C, typename R, typename T1 >
	struct function_info<R (C::*)(T1) const, NullType>  : function_type<R (C::*)(T1) const>, public std::binary_function<const C*, T1, R>
	{
		typedef R							RetType;
		typedef typename TypeList<const C*, T1>::type	ParamTypes;
		typedef R Signature(const C*, T1);
	};

	template < typename R, typename T1, typename T2 > struct function_type<R(T1, T2), NullType>
	{ static const FunctionType::Enum Type = FunctionType::RawFunction; };
	template < typename R, typename T1, typename T2 > struct function_type<R(*)(T1, T2), NullType>
	{ static const FunctionType::Enum Type = FunctionType::RawFunctionPtr; };
	template < typename C, typename R, typename T1, typename T2 > struct function_type<R (C::*)(T1, T2), NullType>
	{ static const FunctionType::Enum Type = FunctionType::MethodPtr; };
	template < typename C, typename R, typename T1, typename T2 > struct function_type<R (C::*)(T1, T2) const, NullType>
	{ static const FunctionType::Enum Type = FunctionType::MethodPtr; };
	template < typename R, typename T1, typename T2 >
	struct function_info<R(T1, T2), NullType> : function_type<R(T1, T2)>, public std::binary_function<T1, T2, R>
	{
		typedef R								RetType;
		typedef typename TypeList<T1, T2>::type	ParamTypes;
		typedef R Signature(T1, T2);
	};
	template < typename R, typename T1, typename T2 >
	struct function_info<R(*)(T1, T2), NullType> : function_type<R(*)(T1, T2)>, public std::binary_function<T1, T2, R>
	{
		typedef R								RetType;
		typedef typename TypeList<T1, T2>::type	ParamTypes;
		typedef R Signature(T1, T2);
	};
	template < typename C, typename R, typename T1, typename T2 >
	struct function_info<R (C::*)(T1, T2), NullType> : function_type<R (C::*)(T1, T2)>
	{
		typedef R									RetType;
		typedef typename TypeList<C*, T1, T2>::type	ParamTypes;
		typedef R Signature(C*, T1, T2);
	};
	template < typename C, typename R, typename T1, typename T2 >
	struct function_info<R (C::*)(T1, T2) const, NullType>  : function_type<R (C::*)(T1, T2) const>
	{
		typedef R											RetType;
		typedef typename TypeList<const C*, T1, T2>::type	ParamTypes;
		typedef R Signature(const C*, T1, T2);
	};

#define DETAIL_TOOLKIT_DECLARE_FUNCTION_INFO(ParamTypenames_, ParamTypes_) \
	template < typename R, ParamTypenames_ > struct function_type<R(ParamTypes_), NullType> \
	{ static const FunctionType::Enum Type = FunctionType::RawFunction; }; \
	template < typename R, ParamTypenames_ > struct function_type<R(*)(ParamTypes_), NullType> \
	{ static const FunctionType::Enum Type = FunctionType::RawFunctionPtr; }; \
	template < typename C, typename R, ParamTypenames_ > struct function_type<R (C::*)(ParamTypes_), NullType> \
	{ static const FunctionType::Enum Type = FunctionType::MethodPtr; }; \
	template < typename C, typename R, ParamTypenames_ > struct function_type<R (C::*)(ParamTypes_) const, NullType> \
	{ static const FunctionType::Enum Type = FunctionType::MethodPtr; }; \
	template < typename R, ParamTypenames_ > \
	struct function_info<R(ParamTypes_), NullType> : function_type<R(ParamTypes_)> \
	{ \
		typedef R										RetType; \
		typedef typename TypeList<ParamTypes_>::type	ParamTypes; \
		typedef R Signature(ParamTypes_); \
	}; \
	template < typename R, ParamTypenames_ > \
	struct function_info<R(*)(ParamTypes_), NullType> : function_type<R(*)(ParamTypes_)> \
	{ \
		typedef R										RetType; \
		typedef typename TypeList<ParamTypes_>::type	ParamTypes; \
		typedef R Signature(ParamTypes_); \
	}; \
	template < typename C, typename R, ParamTypenames_ > \
	struct function_info<R (C::*)(ParamTypes_), NullType> : function_type<R (C::*)(ParamTypes_)> \
	{ \
		typedef R											RetType; \
		typedef typename TypeList<C*, ParamTypes_>::type	ParamTypes; \
		typedef R Signature(C*, ParamTypes_); \
	}; \
	template < typename C, typename R, ParamTypenames_ > \
	struct function_info<R (C::*)(ParamTypes_) const, NullType>  : function_type<R (C::*)(ParamTypes_) const> \
	{ \
		typedef R												RetType; \
		typedef typename TypeList<const C*, ParamTypes_>::type	ParamTypes; \
		typedef R Signature(const C*, ParamTypes_); \
	}

	DETAIL_TOOLKIT_DECLARE_FUNCTION_INFO(MK_PARAM(TY T1, TY T2, TY T3), MK_PARAM(T1, T2, T3));
	DETAIL_TOOLKIT_DECLARE_FUNCTION_INFO(MK_PARAM(TY T1, TY T2, TY T3, TY T4), MK_PARAM(T1, T2, T3, T4));
	DETAIL_TOOLKIT_DECLARE_FUNCTION_INFO(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5), MK_PARAM(T1, T2, T3, T4, T5));
	DETAIL_TOOLKIT_DECLARE_FUNCTION_INFO(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6), MK_PARAM(T1, T2, T3, T4, T5, T6));
	DETAIL_TOOLKIT_DECLARE_FUNCTION_INFO(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7), MK_PARAM(T1, T2, T3, T4, T5, T6, T7));
	DETAIL_TOOLKIT_DECLARE_FUNCTION_INFO(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8));
	DETAIL_TOOLKIT_DECLARE_FUNCTION_INFO(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9));
	DETAIL_TOOLKIT_DECLARE_FUNCTION_INFO(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10));

#undef DETAIL_TOOLKIT_DECLARE_FUNCTION_INFO


///////////////////////////
// for toolkit::function //
///////////////////////////

	template < typename Signature_ >
	class function;

	template < typename Signature_ >
	struct function_type<function<Signature_>, NullType >
	{ static const FunctionType::Enum Type = FunctionType::GSFunction; };


	template < typename Signature_ >
	struct function_info<function<Signature_>, NullType > : public function_type<function<Signature_> >
	{
		typedef typename function_info<Signature_>::RetType		RetType;
		typedef typename function_info<Signature_>::ParamTypes	ParamTypes;
		typedef typename function_info<Signature_>::Signature	Signature;
	};


//////////////////////////////////////////////
// assuming that all other are stl functors //
//////////////////////////////////////////////

	namespace Detail
	{

		template < size_t ParamsCount, typename RetType_, typename ParamTypes_ >
		struct SignatureBuilderImpl;

#define DETAIL_TOOLKIT_DECLARE_SIGNATURE_BUILDER_ENUM_PARAMS(ParamNumber_, TypeListName_) TOOLKIT_COMMA_IF(ParamNumber_) typename GetTypeListItem<TypeListName_, ParamNumber_>::ValueT
#define DETAIL_TOOLKIT_DECLARE_SIGNATURE_BUILDER(ParamsCount_, UserData_) \
		template < typename RetType_, typename ParamTypes_ > \
		struct SignatureBuilderImpl<ParamsCount_, RetType_, ParamTypes_> \
		{ \
			typedef RetType_ 		ValueT(TOOLKIT_REPEAT(ParamsCount_, DETAIL_TOOLKIT_DECLARE_SIGNATURE_BUILDER_ENUM_PARAMS, ParamTypes_)); \
		};

		TOOLKIT_REPEAT_NESTING_2(10, DETAIL_TOOLKIT_DECLARE_SIGNATURE_BUILDER, ~)

#undef DETAIL_TOOLKIT_DECLARE_SIGNATURE_BUILDER
#undef DETAIL_TOOLKIT_DECLARE_SIGNATURE_BUILDER_ENUM_PARAMS


		template < typename RetType_, typename ParamTypes_ >
		struct SignatureBuilder
		{ typedef typename SignatureBuilderImpl<GetTypeListLength<ParamTypes_>::Value, RetType_, ParamTypes_>::ValueT ValueT; };


		template < typename RetType_>
		struct SignatureBuilder<RetType_, UnspecifiedParamTypes>
		{ typedef NullType ValueT; };


		template < typename RetType_>
		struct SignatureBuilder<RetType_, NullType>
		{ typedef NullType ValueT; };


		template < typename F >
		class GetStlFunctorNumArguments
		{
		private:
			struct _wtf_ { char dummy[2]; };
			struct _1_ { _wtf_ dummy[2]; };
			struct _2_ { _1_ dummy[2]; };

			template < typename R, typename T1, typename T2 >
			static _2_ Test(const std::binary_function<T1, T2, R>*);

			template < typename R, typename T1 >
			static _1_ Test(const std::unary_function<T1, R>*);

			static _wtf_ Test(...);
			CompileTimeAssert<sizeof(_1_) != sizeof(_2_) && sizeof(_2_) != sizeof(_wtf_) && sizeof(_1_) != sizeof(_wtf_)> ERROR__type_detectors_dont_work;
		public:
			static const int Value =
				(sizeof(Test((const F*)0)) == sizeof(_1_)
					? 1
					: (sizeof(Test((const F*)0)) == sizeof(_2_)
						? 2
						: -1
					)
				);
		};

		template < typename RetType_, typename ParamTypes_, size_t ParamsCount = GetTypeListLength<ParamTypes_>::Value >
		struct StdSomethingnaryFunctionMixin
		{ };

		template < typename RetType_, typename ParamTypes_ >
		struct StdSomethingnaryFunctionMixin<RetType_, ParamTypes_, 1> : public std::unary_function<typename GetTypeListItem<ParamTypes_, 0>::ValueT, RetType_>
		{ };

		template < typename RetType_, typename ParamTypes_ >
		struct StdSomethingnaryFunctionMixin<RetType_, ParamTypes_, 2> : public std::binary_function<typename GetTypeListItem<ParamTypes_, 0>::ValueT, typename GetTypeListItem<ParamTypes_, 1>::ValueT, RetType_>
		{ };

		template < typename F, int StlFunctorNumArguments = GetStlFunctorNumArguments<F>::Value >
		struct std_function_type;

		template < typename F, int StlFunctorNumArguments = GetStlFunctorNumArguments<F>::Value >
		struct std_function_info;

		template < typename F> struct std_function_type<F, 1>
		{ static const FunctionType::Enum Type = FunctionType::StdFunction; };

		template < typename F >
		struct std_function_info<F, 1> : public std_function_type<F>, public std::unary_function<typename F::argument_type, typename F::result_type>
		{
			typedef typename F::result_type									RetType;
			typedef typename TypeList<typename F::argument_type>::type		ParamTypes;
			typedef typename SignatureBuilder<RetType, ParamTypes>::ValueT	Signature;
		};

		template < typename F> struct std_function_type<F, 2>
		{ static const FunctionType::Enum Type = FunctionType::StdFunction; };

		template < typename F >
		struct std_function_info<F, 2> : public std_function_type<F>, public std::binary_function<typename F::first_argument_type, typename F::second_argument_type, typename F::result_type>
		{
			typedef typename F::result_type									RetType;
			typedef typename TypeList<typename F::first_argument_type,
							 typename F::second_argument_type>::type		ParamTypes;
			typedef typename SignatureBuilder<RetType, ParamTypes>::ValueT	Signature;
		};


		template < typename F >
		struct std_function_type<F, -1>
		{ static const FunctionType::Enum Type = FunctionType::Other; };

		TOOLKIT_DECLARE_NESTED_TYPE_CHECK(RetType);
		TOOLKIT_DECLARE_NESTED_TYPE_CHECK(ParamTypes);

		template < typename F, bool HasParamTypes = HasNestedType_ParamTypes<F>::Value >
		struct GenericFunctionInfo
		{
			typedef typename F::RetType										RetType;
			typedef typename F::ParamTypes									ParamTypes;
			typedef typename SignatureBuilder<RetType, ParamTypes>::ValueT	Signature;
		};

		template < typename F >
		struct GenericFunctionInfo<F, false>
		{
			typedef typename F::RetType	RetType;
			typedef NullType			ParamTypes;
			typedef NullType			Signature;
		};

		template < typename F >
		struct std_function_info<F, -1> : public std_function_type<F>
		{
			typedef typename GenericFunctionInfo<F>::RetType	RetType;
			typedef typename GenericFunctionInfo<F>::ParamTypes	ParamTypes;
			typedef typename GenericFunctionInfo<F>::Signature	Signature;
		};
	}

	template < typename F >
	struct function_type<F, NullType> : public Detail::std_function_type<F>
	{ };


	template < typename F >
	struct function_info<F, NullType> : public Detail::std_function_info<F>
	{ };


	template < typename RetType_, typename ParamTypes_ >
	struct function_type
	{ static const FunctionType::Enum Type = FunctionType::Other; };


	template < typename RetType_>
	struct function_info<RetType_, UnspecifiedParamTypes> : public function_type<RetType_, UnspecifiedParamTypes>
	{
		typedef RetType_		RetType;
	};


	template < typename RetType_, typename ParamTypes_ >
	struct function_info : public function_type<RetType_, ParamTypes_>, public Detail::StdSomethingnaryFunctionMixin<RetType_, ParamTypes_>
	{
		typedef RetType_															RetType;
		typedef ParamTypes_															ParamTypes;
		typedef typename Detail::SignatureBuilder<RetType_, ParamTypes_>::ValueT	Signature;
	};


#undef TY

#else

	/**
	 * @brief Function object parser
	 * @par Example:
	 * @code
	 * typedef function_info<int(char, double, int)>	the_func_info;
	 * if (TypeListContains<the_func_info::ParamTypes, the_func_info::RetType>::Value)
	 *     std::cout << "The parameters list of the function contains its return type" << std::endl;
	 * @endcode
	 */
	template < typename FuncOrRetType, typename OptionalParamTypes = NullType >
	struct function_info
	{
		/** @brief The return type of the function */
		typedef function_ret_type		RetType;
		/** @brief A typelist containing function parameters' types*/
		typedef function_param_types	ParamTypes;
	};

	/**
	 * @brief Function object type getter
	 * @par Example:
	 * @code
	 * if (function_type<int(*)(char, double, int)>::Type == FunctionType::RawFunctionPtr)
	 *     std::cout << "The type of the function is RawFunctionPtr" << std::endl;
	 * @endcode
	 */
	template < typename FuncOrRetType, typename OptionalParamTypes = NullType >
	struct function_type
	{ static const FunctionType::Enum Type = type_of_the_function; };

#endif

	/** @} */

}



#endif
