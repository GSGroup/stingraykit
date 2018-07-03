#ifndef STINGRAYKIT_FUNCTION_FUNCTION_NAME_GETTER_H
#define STINGRAYKIT_FUNCTION_FUNCTION_NAME_GETTER_H

// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/function/function_info.h>
#include <stingraykit/TypeInfo.h>

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


	STINGRAYKIT_DECLARE_METHOD_CHECK(get_name);


	template < typename Func_, typename FunctionType::Enum FuncType_ = function_type<Func_>::Type, bool HasGetName = HasMethod_get_name<Func_>::Value >
	struct function_name_getter
	{
		static std::string get(const Func_& func) { return "{ functor: " + TypeInfo(func).GetName() + " }"; }
	};


	template < typename Func_ >
	struct function_name_getter<Func_, FunctionType::RawFunctionPtr, false>
	{
		static std::string get(const Func_& func) { return "{ function: " + Detail::FuncPtrToString((intptr_t*)(&func), sizeof(func) / sizeof(intptr_t)) + " }"; }
	};


	template < typename Func_ >
	struct function_name_getter<Func_, FunctionType::MethodPtr, false>
	{
		static std::string get(const Func_& func) { return "{ method: " + Detail::FuncPtrToString((intptr_t*)(&func), sizeof(func) / sizeof(intptr_t)) + " }"; }
	};


	template < typename Func_, typename FunctionType::Enum FuncType_ >
	struct function_name_getter<Func_, FuncType_, true>
	{
		static std::string get(const Func_& func) { return func.get_name(); }
	};


	template < typename Func_ >
	std::string get_function_name(const Func_& func)
	{ return function_name_getter<Func_>::get(func); }

#endif

	/** @} */

}

#endif
