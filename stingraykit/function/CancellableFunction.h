#ifndef STINGRAYKIT_FUNCTION_CANCELLABLEFUNCTION_H
#define STINGRAYKIT_FUNCTION_CANCELLABLEFUNCTION_H

#include <stingraykit/PerfectForwarding.h>

namespace stingray
{

	template< typename FunctorType >
	class CancellableFunction : public function_info<FunctorType>
	{
	private:
		FunctorType				_func;
		FutureExecutionTester	_tester;

	public:
		CancellableFunction(const FunctorType& func, const FutureExecutionTester& tester) :
			_func(func), _tester(tester)
		{ }

		STINGRAYKIT_PERFECT_FORWARDING(void, operator (), Do)

		std::string get_name() const
		{ return "{ CancellableFunction: " + get_function_name(_func) + " }"; }

	private:
		template< typename ParamTypeList >
		void Do(const Tuple<ParamTypeList>& params) const
		{
			LocalExecutionGuard guard(_tester);
			if (guard)
				FunctorInvoker::Invoke(_func, params);
		}
	};


	template < typename FuncType >
	CancellableFunction<FuncType> MakeCancellableFunction(const FuncType& func, const FutureExecutionTester& tester)
	{ return CancellableFunction<FuncType>(func, tester); }

}

#endif
