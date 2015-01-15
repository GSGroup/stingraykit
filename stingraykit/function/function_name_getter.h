#ifndef STINGRAYKIT_FUNCTION_FUNCTION_NAME_GETTER_H
#define STINGRAYKIT_FUNCTION_FUNCTION_NAME_GETTER_H


#include <stdio.h>

#include <typeinfo>

//#include <stingraykit/string/StringUtils.h>
#include <stingraykit/toolkit.h>
#include <stingraykit/function/function_info.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

#ifndef DOXYGEN_PREPROCESSOR

	namespace Detail
	{
		std::string FuncPtrToString(intptr_t* ptrptr, size_t n);
	}

	template < typename Func_, typename FunctionType::Enum FuncType_ = function_type<Func_>::Type >
	struct function_name_getter
	{
		static std::string get(const Func_& func) { return "{ functor: " + Demangle(typeid(func).name()) + " }"; }
	};

	template < typename Func_ >
	struct function_name_getter<Func_, FunctionType::RawFunctionPtr>
	{
		static std::string get(const Func_& func) { return "{ function: " + Detail::FuncPtrToString((intptr_t*)(&func), sizeof(func) / sizeof(intptr_t)) + " }"; }
	};

	template < typename Func_ >
	struct function_name_getter<Func_, FunctionType::MethodPtr>
	{
		static std::string get(const Func_& func) { return "{ method: " + Detail::FuncPtrToString((intptr_t*)(&func), sizeof(func) / sizeof(intptr_t)) + " }"; }
	};

	template < typename Func_ >
	struct function_name_getter<Func_, FunctionType::GSFunction>
	{
		static std::string get(const Func_& func) { return "{ gs_function: " + func.get_name() + " }"; }
	};

	template < typename Signature >
	class function_with_token;

	template < typename Signature, typename FunctionType::Enum FuncType_ >
	struct function_name_getter<function_with_token<Signature>, FuncType_>
	{
		static std::string get(const function_with_token<Signature>& func) { return "{ function_with_token: " + func.get_name() + " }"; }
	};


	namespace Detail
	{
		template
			<
				typename RetType,
				typename AllParameters,
				typename FunctorType,
				size_t BinderParamsCount
			>
		struct Binder;
	}

	template
		<
			typename RetType,
			typename AllParameters,
			typename FunctorType,
			size_t BinderParamsCount,
			typename FunctionType::Enum FuncType_
		>
	struct function_name_getter<Detail::Binder<RetType, AllParameters, FunctorType, BinderParamsCount>, FuncType_>
	{
		static std::string get(const Detail::Binder<RetType, AllParameters, FunctorType, BinderParamsCount>& func) { return "{ binder: " + func.GetFuncName() + " }"; }
	};


	template < typename Func_ >
	std::string get_function_name(const Func_& func)
	{ return function_name_getter<Func_>::get(func); }

#endif

	/** @} */

}

#endif
