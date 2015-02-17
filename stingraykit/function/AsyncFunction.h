#ifndef STINGRAYKIT_FUNCTION_ASYNCFUNCTION_H
#define STINGRAYKIT_FUNCTION_ASYNCFUNCTION_H

#include <stingraykit/Macro.h>
#include <stingraykit/function/CancellableFunction.h>
#include <stingraykit/function/bind.h>
#include <stingraykit/function/function.h>
#include <stingraykit/future.h>
#include <stingraykit/thread/ITaskExecutor.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

	namespace Detail
	{
		template <typename FunctorType, bool HasReturnType = !SameType<typename function_info<FunctorType>::RetType, void>::Value>
		class AsyncFunctionBase
		{
		public:
			typedef void											RetType;
			typedef typename function_info<FunctorType>::ParamTypes	ParamTypes;

		protected:
			ITaskExecutorPtr	_executor;
			FunctorType			_func;
			TaskLifeToken		_token;

		protected:
			AsyncFunctionBase(const ITaskExecutorPtr& executor, const FunctorType& func) :
				_executor(STINGRAYKIT_REQUIRE_NOT_NULL(executor)), _func(func)
			{ }

			AsyncFunctionBase(const ITaskExecutorPtr& executor, const FunctorType& func, const TaskLifeToken& token) :
				_executor(STINGRAYKIT_REQUIRE_NOT_NULL(executor)), _func(func), _token(token)
			{ }

			~AsyncFunctionBase()
			{ }

			template <typename BoundFunctor>
			RetType DoAddTask(const BoundFunctor& func) const
			{ _executor->AddTask(func, _token.GetExecutionTester()); }
		};


		template <typename FunctorType>
		class AsyncFunctionBase<FunctorType, true>
		{
		public:
			typedef typename function_info<FunctorType>::RetType		AsyncRetType;
			typedef promise<AsyncRetType>								PromiseType;
			STINGRAYKIT_DECLARE_PTR(PromiseType);

			typedef future<AsyncRetType>								RetType;
			typedef typename function_info<FunctorType>::ParamTypes		ParamTypes;

		protected:
			ITaskExecutorPtr	_executor;
			FunctorType			_func;
			TaskLifeToken		_token;

		protected:
			AsyncFunctionBase(const ITaskExecutorPtr& executor, const FunctorType& func) :
				_executor(STINGRAYKIT_REQUIRE_NOT_NULL(executor)), _func(func)
			{ }

			~AsyncFunctionBase()
			{ }

			template <typename BoundFunctor>
			RetType DoAddTask(const BoundFunctor& func) const
			{
				PromiseTypePtr promise(new PromiseType);
				_executor->AddTask(bind(&AsyncFunctionBase::FuncWrapper<BoundFunctor>, func, promise), _token.GetExecutionTester());
				return promise->get_future();
			}

		private:
			template <typename BoundFunctor>
			static void FuncWrapper(const BoundFunctor& func, const PromiseTypePtr& promise)
			{
				STINGRAYKIT_REQUIRE_NOT_NULL(promise);
				try
				{ promise->set_value(func()); }
				catch (const std::exception& ex)
				{ promise->set_exception(make_exception_ptr(ex)); }
			}
		};
	}


	template <typename FunctorType>
	class AsyncFunction : public Detail::AsyncFunctionBase<FunctorType>
	{
		typedef Detail::AsyncFunctionBase<FunctorType>	base;

	public:
		AsyncFunction(const ITaskExecutorPtr& executor, const FunctorType& func) :
			base(executor, func)
		{ }

		AsyncFunction(const ITaskExecutorPtr& executor, const FunctorType& func, const TaskLifeToken& token) :
			base(executor, func, token)
		{ }

		TaskLifeToken GetToken() const { return this->_token; }

		STINGRAYKIT_CONST_FORWARDING(typename base::RetType, operator (), Do)

		std::string get_name() const
		{ return "{ AsyncFunction: " + get_function_name(this->_func) + " }"; }

	private:
		template< typename ParamTypeList >
		typename base::RetType Do(const Tuple<ParamTypeList>& params) const
		{ return base::DoAddTask(Detail::Binder<typename TypeListTransform<ParamTypeList, Dereference>::ValueT, FunctorType>(this->_func, params)); }
	};


	template <typename FunctorType>
	AsyncFunction<FunctorType> MakeAsyncFunction(const ITaskExecutorPtr& executor, const FunctorType& func)
	{ return AsyncFunction<FunctorType>(executor, func); }


	template <typename FunctorType>
	AsyncFunction<FunctorType> MakeAsyncFunction(const ITaskExecutorPtr& executor, const FunctorType& func, const TaskLifeToken& token)
	{ return AsyncFunction<FunctorType>(executor, func, token); }

	/** @} */

}



#endif
