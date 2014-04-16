#ifndef STINGRAY_TOOLKIT_CANCELLABLEFUNCTION_H
#define STINGRAY_TOOLKIT_CANCELLABLEFUNCTION_H

namespace stingray
{

	template<typename Signature>
	class CancellableFunction;


#define DETAIL_TOOLKIT_DECLARE_CANCELLABLE_FUNCTION(ParamsCount_, UserData_) \
	template < TOOLKIT_REPEAT(ParamsCount_, TOOLKIT_TEMPLATE_PARAM_DECL, T) > \
	class CancellableFunction<void(TOOLKIT_REPEAT(ParamsCount_, TOOLKIT_TEMPLATE_PARAM_USAGE, T))> : public function_info<void(TOOLKIT_REPEAT(ParamsCount_, TOOLKIT_TEMPLATE_PARAM_USAGE, T))> \
	{ \
	private: \
		function<void(TOOLKIT_REPEAT(ParamsCount_, TOOLKIT_TEMPLATE_PARAM_USAGE, T))>	_func; \
		FutureExecutionTester															_tester; \
	 \
	public: \
		CancellableFunction(const function<void(TOOLKIT_REPEAT(ParamsCount_, TOOLKIT_TEMPLATE_PARAM_USAGE, T))>& func, const FutureExecutionTester& tester) : \
			_func(func), _tester(tester) \
		{ } \
		\
		void operator ()(TOOLKIT_REPEAT(ParamsCount_, TOOLKIT_FUNCTION_PARAM_DECL, T)) const \
		{  \
			LocalExecutionGuard guard; \
			if (_tester.Execute(guard)) \
				_func(TOOLKIT_REPEAT(ParamsCount_, TOOLKIT_FUNCTION_PARAM_USAGE, T)); \
		} \
	};

	TOOLKIT_REPEAT_NESTING_2(30, DETAIL_TOOLKIT_DECLARE_CANCELLABLE_FUNCTION, ~)

#undef DETAIL_TOOLKIT_DECLARE_CANCELLABLE_FUNCTION

	template < typename FuncType >
	CancellableFunction<typename function_info<FuncType>::Signature> MakeCancellableFunction(const FuncType& func, const FutureExecutionTester& tester)
	{ return CancellableFunction<typename function_info<FuncType>::Signature>(func, tester); }

}

#endif
