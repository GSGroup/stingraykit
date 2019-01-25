#ifndef STINGRAYKIT_FUNCTION_FUNCTORINVOKER_H
#define STINGRAYKIT_FUNCTION_FUNCTORINVOKER_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
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

		template < size_t ParametersCount, bool IsMethod >
		struct FunctorInvokerImpl;

		template < > struct FunctorInvokerImpl<0, false>
		{
			template < typename FunctorType, typename ParametersTuple >
			static inline typename function_info<FunctorType>::RetType Invoke(const FunctorType& func, const ParametersTuple& p)
			{
				//CompileTimeAssert<ParametersTuple::Size == 0> ERROR__invalid_number_of_parameters;
				return func();
			}
		};

#define DETAIL_STINGRAYKIT_DECLARE_FUNCTOR_INVOKER_IMPL(N, ...) \
		template < > struct FunctorInvokerImpl<N, false> \
		{ \
			template < typename FunctorType, typename ParametersTuple > \
			static inline typename function_info<FunctorType>::RetType Invoke(const FunctorType& func, const ParametersTuple& p) \
			{ \
				/*CompileTimeAssert<ParametersTuple::Size == 0> ERROR__invalid_number_of_parameters;*/ \
				return func(p.template Get<0>(), ##__VA_ARGS__);  \
			} \
		}; \
		template < > struct FunctorInvokerImpl<N, true> \
		{ \
			template < typename FunctorType, typename ParametersTuple > \
			static inline typename function_info<FunctorType>::RetType Invoke(const FunctorType& func, const ParametersTuple& p) \
			{ \
				/*CompileTimeAssert<ParametersTuple::Size == 0> ERROR__invalid_number_of_parameters;*/ \
				return (STINGRAYKIT_REQUIRE_NOT_NULL(to_pointer(p.template Get<0>()))->*func)(__VA_ARGS__);  \
			} \
		}

#define P(N) p.template Get<N>()

		DETAIL_STINGRAYKIT_DECLARE_FUNCTOR_INVOKER_IMPL(1);
		DETAIL_STINGRAYKIT_DECLARE_FUNCTOR_INVOKER_IMPL(2, MK_PARAM(P(1)));
		DETAIL_STINGRAYKIT_DECLARE_FUNCTOR_INVOKER_IMPL(3, MK_PARAM(P(1), P(2)));
		DETAIL_STINGRAYKIT_DECLARE_FUNCTOR_INVOKER_IMPL(4, MK_PARAM(P(1), P(2), P(3)));
		DETAIL_STINGRAYKIT_DECLARE_FUNCTOR_INVOKER_IMPL(5, MK_PARAM(P(1), P(2), P(3), P(4)));
		DETAIL_STINGRAYKIT_DECLARE_FUNCTOR_INVOKER_IMPL(6, MK_PARAM(P(1), P(2), P(3), P(4), P(5)));
		DETAIL_STINGRAYKIT_DECLARE_FUNCTOR_INVOKER_IMPL(7, MK_PARAM(P(1), P(2), P(3), P(4), P(5), P(6)));
		DETAIL_STINGRAYKIT_DECLARE_FUNCTOR_INVOKER_IMPL(8, MK_PARAM(P(1), P(2), P(3), P(4), P(5), P(6), P(7)));
		DETAIL_STINGRAYKIT_DECLARE_FUNCTOR_INVOKER_IMPL(9, MK_PARAM(P(1), P(2), P(3), P(4), P(5), P(6), P(7), P(8)));
		DETAIL_STINGRAYKIT_DECLARE_FUNCTOR_INVOKER_IMPL(10, MK_PARAM(P(1), P(2), P(3), P(4), P(5), P(6), P(7), P(8), P(9)));

#undef P

	}


	struct FunctorInvoker
	{
		template < typename FunctorType, typename ParamsTuple >
		static inline typename function_info<FunctorType>::RetType Invoke(const FunctorType& func, const ParamsTuple& p)
		{
			return Detail::FunctorInvokerImpl
				<
					ParamsTuple::Size,
					/*GetTypeListLength<typename function_info<FunctorType>::ParamTypes>::Value,*/
					function_info<FunctorType>::Type == FunctionType::MethodPtr
				>::template Invoke<FunctorType, ParamsTuple>(func, p);
		}
	};

	/** @} */

}

#endif
