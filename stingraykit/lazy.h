#ifndef STINGRAYKIT_LAZY_H
#define STINGRAYKIT_LAZY_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/function/FunctorInvoker.h>

namespace stingray
{
	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

	namespace Detail
	{
		template < typename FuncType >
		class LazyVal
		{
		private:
			FuncType	_func;

		public:
			LazyVal(const FuncType& func)
				: _func(func)
			{ }

			LazyVal(FuncType&& func)
				: _func(std::move(func))
			{ }

			operator typename function_info<FuncType>::RetType () const
			{ return FunctorInvoker::InvokeArgs(_func); }
		};
	}


	template < typename FuncType >
	Detail::LazyVal<typename Decay<FuncType>::ValueT> lazy(FuncType&& func)
	{ return Detail::LazyVal<typename Decay<FuncType>::ValueT>(std::forward<FuncType>(func)); }

	/** @} */
}

#endif
