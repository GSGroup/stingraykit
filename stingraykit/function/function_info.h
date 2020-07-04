#ifndef STINGRAYKIT_FUNCTION_FUNCTION_INFO_H
#define STINGRAYKIT_FUNCTION_FUNCTION_INFO_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/function/SignatureBuilder.h>
#include <stingraykit/Macro.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

	struct FunctionType
	{
		STINGRAYKIT_ENUM_VALUES
		(
			RawFunction,
			RawFunctionPtr,
			MethodPtr,
			StingrayFunction,
			StdFunction,
			Other
		);

		STINGRAYKIT_DECLARE_ENUM_CLASS(FunctionType);
	};


#ifndef DOXYGEN_PREPROCESSOR

#define TY typename

	template < typename FuncOrRetType, typename OptionalParamTypes = NullType >
	struct function_type;


	template < typename FuncOrRetType, typename OptionalParamTypes = NullType >
	struct function_info;


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
		typedef R					RetType;
		typedef TypeList<>::type	ParamTypes;
		typedef R Signature();
	};
	template < typename R >
	struct function_info<R(*)(), NullType> : public function_type<R(*)()>
	{
		typedef R					RetType;
		typedef TypeList<>::type	ParamTypes;
		typedef R Signature();
	};
	template < typename C, typename R >
	struct function_info<R (C::*)(), NullType> : public function_type<R (C::*)()>
	{
		typedef R				RetType;
		typedef typename TypeList<C*>::type	ParamTypes;
		typedef R Signature(C*);
	};
	template < typename C, typename R >
	struct function_info<R (C::*)() const, NullType> : public function_type<R (C::*)() const>
	{
		typedef R						RetType;
		typedef typename TypeList<const C*>::type	ParamTypes;
		typedef R Signature(const C*);
	};


#define DETAIL_STINGRAYKIT_DECLARE_FUNCTION_INFO(ParamTypenames_, ParamTypes_) \
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
	struct function_info<R (C::*)(ParamTypes_) const, NullType> : function_type<R (C::*)(ParamTypes_) const> \
	{ \
		typedef R												RetType; \
		typedef typename TypeList<const C*, ParamTypes_>::type	ParamTypes; \
		typedef R Signature(const C*, ParamTypes_); \
	}

	DETAIL_STINGRAYKIT_DECLARE_FUNCTION_INFO(MK_PARAM(TY T1), MK_PARAM(T1));
	DETAIL_STINGRAYKIT_DECLARE_FUNCTION_INFO(MK_PARAM(TY T1, TY T2), MK_PARAM(T1, T2));
	DETAIL_STINGRAYKIT_DECLARE_FUNCTION_INFO(MK_PARAM(TY T1, TY T2, TY T3), MK_PARAM(T1, T2, T3));
	DETAIL_STINGRAYKIT_DECLARE_FUNCTION_INFO(MK_PARAM(TY T1, TY T2, TY T3, TY T4), MK_PARAM(T1, T2, T3, T4));
	DETAIL_STINGRAYKIT_DECLARE_FUNCTION_INFO(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5), MK_PARAM(T1, T2, T3, T4, T5));
	DETAIL_STINGRAYKIT_DECLARE_FUNCTION_INFO(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6), MK_PARAM(T1, T2, T3, T4, T5, T6));
	DETAIL_STINGRAYKIT_DECLARE_FUNCTION_INFO(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7), MK_PARAM(T1, T2, T3, T4, T5, T6, T7));
	DETAIL_STINGRAYKIT_DECLARE_FUNCTION_INFO(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8));
	DETAIL_STINGRAYKIT_DECLARE_FUNCTION_INFO(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9));
	DETAIL_STINGRAYKIT_DECLARE_FUNCTION_INFO(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10));

#undef DETAIL_STINGRAYKIT_DECLARE_FUNCTION_INFO


////////////////////////////
// for stingray::function //
////////////////////////////

	template < typename Signature_ >
	class function;

	template < typename Signature_ >
	struct function_type<function<Signature_>, NullType>
	{ static const FunctionType::Enum Type = FunctionType::StingrayFunction; };


	template < typename Signature_ >
	struct function_info<function<Signature_>, NullType> : public function_type<function<Signature_> >
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
		STINGRAYKIT_DECLARE_NESTED_TYPE_CHECK(RetType);
		STINGRAYKIT_DECLARE_NESTED_TYPE_CHECK(ParamTypes);
		STINGRAYKIT_DECLARE_NESTED_TYPE_CHECK(result_type);

		struct FunctorType
		{
			STINGRAYKIT_ENUM_VALUES(FunctorWithRetAndParamTypes, FunctorWithRetType, StdFunctor, Other);
			STINGRAYKIT_DECLARE_ENUM_CLASS(FunctorType);
		};

		template < typename F, FunctorType::Enum FuncType =
			HasNestedType_RetType<F>::Value ? (HasNestedType_ParamTypes<F>::Value ? FunctorType::FunctorWithRetAndParamTypes : FunctorType::FunctorWithRetType) :
				HasNestedType_result_type<F>::Value ? FunctorType::StdFunctor : FunctorType::Other>
		struct FunctorTypeGetter
		{ static const FunctorType::Enum Value = FuncType; };


		template < typename F, FunctorType::Enum FuncType = FunctorTypeGetter<F>::Value >
		struct GenericFunctionType;

		template < typename F >
		struct GenericFunctionType<F, FunctorType::FunctorWithRetAndParamTypes>
		{ static const FunctionType::Enum Type = FunctionType::StingrayFunction; };

		template < typename F >
		struct GenericFunctionType<F, FunctorType::FunctorWithRetType>
		{ static const FunctionType::Enum Type = FunctionType::StingrayFunction; };

		template < typename F >
		struct GenericFunctionType<F, FunctorType::StdFunctor>
		{ static const FunctionType::Enum Type = FunctionType::StdFunction; };

		template < typename F >
		struct GenericFunctionType<F, FunctorType::Other>
		{ static const FunctionType::Enum Type = FunctionType::Other; };


		template < typename F, FunctorType::Enum FuncType = FunctorTypeGetter<F>::Value >
		struct GenericFunctionInfo;

		template < typename F >
		struct GenericFunctionInfo<F, FunctorType::FunctorWithRetAndParamTypes>
		{
			typedef typename F::RetType										RetType;
			typedef typename F::ParamTypes									ParamTypes;
			typedef typename SignatureBuilder<RetType, ParamTypes>::ValueT	Signature;
		};

		template < typename F >
		struct GenericFunctionInfo<F, FunctorType::FunctorWithRetType>
		{
			typedef typename F::RetType		RetType;
			typedef UnspecifiedParamTypes	ParamTypes;
			typedef NullType				Signature;
		};

		template < typename F >
		struct GenericFunctionInfo<F, FunctorType::StdFunctor>
		{
			typedef typename F::result_type	RetType;
			typedef UnspecifiedParamTypes	ParamTypes;
			typedef NullType				Signature;
		};

		template < typename F >
		struct GenericFunctionInfo<F, FunctorType::Other>
		{
			typedef UnspecifiedRetType		RetType;
			typedef UnspecifiedParamTypes	ParamTypes;
			typedef NullType				Signature;
		};
	}


	template < typename F >
	struct function_type<F, NullType> : public Detail::GenericFunctionType<F>
	{ };


	template < typename F >
	struct function_info<F, NullType> : public Detail::GenericFunctionInfo<F>
	{ };


	template < typename RetType_, typename ParamTypes_ >
	struct function_type
	{ static const FunctionType::Enum Type = FunctionType::Other; };


	template < typename RetType_, typename ParamTypes_ >
	struct function_info : public function_type<RetType_, ParamTypes_>
	{
		typedef RetType_													RetType;
		typedef ParamTypes_													ParamTypes;
		typedef typename SignatureBuilder<RetType_, ParamTypes_>::ValueT	Signature;
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
		/** @brief Function signature as single type*/
		typedef function_signature		Signature;
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
