#ifndef STINGRAY_TOOLKIT_LAZY_H
#define STINGRAY_TOOLKIT_LAZY_H


#include <stingray/toolkit/function/function.h>


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
