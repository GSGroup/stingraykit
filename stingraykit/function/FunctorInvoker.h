#ifndef STINGRAYKIT_FUNCTION_FUNCTORINVOKER_H
#define STINGRAYKIT_FUNCTION_FUNCTORINVOKER_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/function/function_info.h>
#include <stingraykit/Tuple.h>
#include <stingraykit/exception.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

	namespace Detail
	{

		template < typename FunctorType, FunctionType::Enum Type = function_type<FunctorType>::Type >
		struct FunctorInvokerImpl
		{
			template < typename ParamsTuple >
			static typename function_info<FunctorType>::RetType Invoke(const FunctorType& func, ParamsTuple&& params)
			{ return InvokeImpl(std::make_index_sequence<Decay<ParamsTuple>::ValueT::Size>(), func, std::forward<ParamsTuple>(params)); }

			template < typename... Ts >
			static typename function_info<FunctorType>::RetType InvokeArgs(const FunctorType& func, Ts&&... args)
			{ return func(std::forward<Ts>(args)...); }

		private:
			template < typename ParamsTuple, size_t... Index >
			static typename function_info<FunctorType>::RetType InvokeImpl(std::index_sequence<Index...>, const FunctorType& func, ParamsTuple&& params)
			{ return InvokeArgs(func, std::forward<ParamsTuple>(params).template Get<Index>()...); }
		};

		template < typename FunctorType >
		struct FunctorInvokerImpl<FunctorType, FunctionType::MethodPtr>
		{
			template < typename ParamsTuple >
			static typename function_info<FunctorType>::RetType Invoke(const FunctorType& func, ParamsTuple&& params)
			{
				static_assert(Decay<ParamsTuple>::ValueT::Size != 0, "Invalid number of parameters");
				return InvokeImpl(std::make_index_sequence<Decay<ParamsTuple>::ValueT::Size>(), func, std::forward<ParamsTuple>(params));
			}

			template < typename This, typename... Ts >
			static typename function_info<FunctorType>::RetType InvokeArgs(const FunctorType& func, This&& thisObj, Ts&&... args)
			{ return (STINGRAYKIT_REQUIRE_NOT_NULL(to_pointer(std::forward<This>(thisObj)))->*func)(std::forward<Ts>(args)...); }

		private:
			template < typename ParamsTuple, size_t... Index >
			static typename function_info<FunctorType>::RetType InvokeImpl(std::index_sequence<Index...>, const FunctorType& func, ParamsTuple&& params)
			{ return InvokeArgs(func, std::forward<ParamsTuple>(params).template Get<Index>()...); }
		};

	}


	struct FunctorInvoker
	{
		template < typename FunctorType, typename ParamsTuple >
		static typename function_info<FunctorType>::RetType Invoke(const FunctorType& func, ParamsTuple&& params)
		{ return Detail::FunctorInvokerImpl<FunctorType>::template Invoke<ParamsTuple>(func, std::forward<ParamsTuple>(params)); }

		template < typename FunctorType, typename... Ts >
		static typename function_info<FunctorType>::RetType InvokeArgs(const FunctorType& func, Ts&&... args)
		{ return Detail::FunctorInvokerImpl<FunctorType>::template InvokeArgs<Ts...>(func, std::forward<Ts>(args)...); }
	};

	/** @} */

}

#endif
