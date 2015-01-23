#ifndef STINGRAYKIT_FUNCTION_FUNCTIONAL_H
#define STINGRAYKIT_FUNCTION_FUNCTIONAL_H


#include <stingraykit/function/function.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

	namespace Detail
	{
		template < typename F >
		class NotFunc : public function_info<F>
		{
			F	_f;

		public:
			NotFunc(const F& f) : _f(f)
			{ }

			STINGRAYKIT_PERFECT_FORWARDING(bool, operator (), Do)

		private:
			template< typename ParamTypeList >
			bool Do(const Tuple<ParamTypeList>& params) const
			{ return !FunctorInvoker::Invoke(_f, params); }
		};
	}

	template < typename F >
	Detail::NotFunc<F> not_(const F& f) { return Detail::NotFunc<F>(f); }

	/** @} */

}


#endif
