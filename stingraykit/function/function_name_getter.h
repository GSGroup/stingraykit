#ifndef STINGRAYKIT_FUNCTION_FUNCTION_NAME_GETTER_H
#define STINGRAYKIT_FUNCTION_FUNCTION_NAME_GETTER_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
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

		template < typename Func_ >
		class FunctionNameGetter
		{
		private:
			const Func_&			_func;

		public:
			explicit FunctionNameGetter(const Func_& func)
				:	_func(func)
			{ }

			std::string ToString() const
			{ return Do(_func, 0); }

		private:
			template < typename Func__ >
			static std::string DoFallback(const Func__& func, typename EnableIf<function_type<Func__>::Type == FunctionType::RawFunctionPtr, int>::ValueT)
			{ return "{ function: " + Detail::FuncPtrToString((intptr_t*)(&func), sizeof(func) / sizeof(intptr_t)) + " }"; }

			template < typename Func__ >
			static std::string DoFallback(const Func__& func, typename EnableIf<function_type<Func__>::Type == FunctionType::MethodPtr, int>::ValueT)
			{ return "{ method: " + Detail::FuncPtrToString((intptr_t*)(&func), sizeof(func) / sizeof(intptr_t)) + " }"; }

			template < typename Func__ >
			static std::string DoFallback(const Func__& func, long)
			{ return "{ functor: " + TypeInfo(func).GetName() + " }"; }

			template < typename Func__ >
			static auto Do(const Func__& func, int)
					-> decltype(func.get_name(), std::string())
			{ return func.get_name(); }

			template < typename Func__ >
			static auto Do(const Func__& func, long)
			{ return DoFallback(func, 0); }
		};
	}


	template < typename Func_ >
	Detail::FunctionNameGetter<Func_> get_function_name(const Func_& func)
	{ return Detail::FunctionNameGetter<Func_>(func); }

	template < typename Func_ >
	Detail::FunctionNameGetter<Func_> get_function_name(const Func_&& func) = delete;

#endif

	/** @} */

}

#endif
