#ifndef STINGRAYKIT_LAZY_H
#define STINGRAYKIT_LAZY_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/function/function.h>


namespace stingray
{
	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

	template < typename T >
	class LazyVal
	{
		typedef function<T()>	Func;

	private:
		Func	_func;

	public:
		LazyVal(const Func& func)
			: _func(func)
		{ }

		operator T() const { return _func(); }
	};


	template < typename FuncType >
	LazyVal<typename function_info<FuncType>::RetType> lazy(const FuncType& func)
	{ return LazyVal<typename function_info<FuncType>::RetType>(func); }

	template < typename T >
	struct ToPointerType<LazyVal<T> >
	{ typedef typename ToPointerType<T>::ValueT ValueT; };

	template < typename T >
	inline typename ToPointerType<LazyVal<T> >::ValueT to_pointer(const LazyVal<T>& lazyPtr) { return to_pointer((T)lazyPtr); }

	/** @} */
}


#endif
