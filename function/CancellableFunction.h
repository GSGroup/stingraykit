#ifndef STINGRAY_STINGRAYKIT_FUNCTION_CANCELLABLEFUNCTION_H
#define STINGRAY_STINGRAYKIT_FUNCTION_CANCELLABLEFUNCTION_H

namespace stingray
{

	template<typename Signature>
	class CancellableFunction;


#define DETAIL_STINGRAYKIT_DECLARE_CANCELLABLE_FUNCTION(ParamsCount_, UserData_) \
	template < STINGRAYKIT_REPEAT(ParamsCount_, STINGRAYKIT_TEMPLATE_PARAM_DECL, T) > \
	class CancellableFunction<void(STINGRAYKIT_REPEAT(ParamsCount_, STINGRAYKIT_TEMPLATE_PARAM_USAGE, T))> : public function_info<void(STINGRAYKIT_REPEAT(ParamsCount_, STINGRAYKIT_TEMPLATE_PARAM_USAGE, T))> \
	{ \
	private: \
		function<void(STINGRAYKIT_REPEAT(ParamsCount_, STINGRAYKIT_TEMPLATE_PARAM_USAGE, T))>	_func; \
		FutureExecutionTester															_tester; \
	 \
	public: \
		CancellableFunction(const function<void(STINGRAYKIT_REPEAT(ParamsCount_, STINGRAYKIT_TEMPLATE_PARAM_USAGE, T))>& func, const FutureExecutionTester& tester) : \
			_func(func), _tester(tester) \
		{ } \
		\
		void operator ()(STINGRAYKIT_REPEAT(ParamsCount_, STINGRAYKIT_FUNCTION_PARAM_DECL, T)) const \
		{  \
			LocalExecutionGuard guard; \
			if (_tester.Execute(guard)) \
				_func(STINGRAYKIT_REPEAT(ParamsCount_, STINGRAYKIT_FUNCTION_PARAM_USAGE, T)); \
		} \
	};

	STINGRAYKIT_REPEAT_NESTING_2(30, DETAIL_STINGRAYKIT_DECLARE_CANCELLABLE_FUNCTION, ~)

#undef DETAIL_STINGRAYKIT_DECLARE_CANCELLABLE_FUNCTION

	template < typename FuncType >
	CancellableFunction<typename function_info<FuncType>::Signature> MakeCancellableFunction(const FuncType& func, const FutureExecutionTester& tester)
	{ return CancellableFunction<typename function_info<FuncType>::Signature>(func, tester); }

}

#endif
