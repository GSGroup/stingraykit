#ifndef STINGRAYKIT_FUNCTION_ASYNCFUNCTION_H
#define STINGRAYKIT_FUNCTION_ASYNCFUNCTION_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/function/bind.h>
#include <stingraykit/thread/ITaskExecutor.h>
#include <stingraykit/future.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

	namespace Detail
	{
		template < typename FunctorType, bool HasReturnType = !IsSame<typename function_info<FunctorType>::RetType, void>::Value >
		class AsyncFunctionBase : public function_info<FunctorType>
		{
		public:
			typedef void											RetType;
			typedef typename function_info<FunctorType>::ParamTypes	ParamTypes;

		protected:
			ITaskExecutorPtr		_executor;
			FunctorType				_func;
			FutureExecutionTester	_tester;

		protected:
			AsyncFunctionBase(const ITaskExecutorPtr& executor, const FunctorType& func, const FutureExecutionTester& tester)
				: _executor(STINGRAYKIT_REQUIRE_NOT_NULL(executor)), _func(func), _tester(tester)
			{ }

			RetType DoAddTask(const function<RetType ()>& func) const
			{ _executor->AddTask(func, _tester); }
		};


		template < typename FunctorType >
		class AsyncFunctionBase<FunctorType, true> : public function_info<FunctorType>
		{
		public:
			typedef typename function_info<FunctorType>::RetType		AsyncRetType;
			typedef promise<AsyncRetType>								PromiseType;
			STINGRAYKIT_DECLARE_PTR(PromiseType);

			typedef future<AsyncRetType>								RetType;
			typedef typename function_info<FunctorType>::ParamTypes		ParamTypes;

		protected:
			ITaskExecutorPtr		_executor;
			FunctorType				_func;
			FutureExecutionTester	_tester;

		protected:
			AsyncFunctionBase(const ITaskExecutorPtr& executor, const FunctorType& func, const FutureExecutionTester& tester)
				: _executor(STINGRAYKIT_REQUIRE_NOT_NULL(executor)), _func(func), _tester(tester)
			{ }

			RetType DoAddTask(const function<AsyncRetType ()>& func) const
			{
				const PromiseTypePtr promise = make_shared_ptr<PromiseType>();
				_executor->AddTask(Bind(&AsyncFunctionBase::FuncWrapper, func, promise), _tester);
				return promise->get_future();
			}

		private:
			static void FuncWrapper(const function<AsyncRetType ()>& func, const PromiseTypePtr& promise)
			{
				try
				{ promise->set_value(func()); }
				catch (const std::exception& ex)
				{ promise->set_exception(MakeExceptionPtr(ex)); }
			}
		};
	}


	template < typename FunctorType >
	class AsyncFunction : public Detail::AsyncFunctionBase<FunctorType>
	{
		typedef Detail::AsyncFunctionBase<FunctorType>	BaseType;

	public:
		AsyncFunction(const ITaskExecutorPtr& executor, const FunctorType& func, const FutureExecutionTester& tester)
			: BaseType(executor, func, tester)
		{ }

		STINGRAYKIT_CONST_FORWARDING(typename BaseType::RetType, operator (), Do)

		std::string get_name() const
		{ return "{ AsyncFunction: " + get_function_name(this->_func) + " }"; }

	private:
		template < typename ParamTypeList >
		typename BaseType::RetType Do(const Tuple<ParamTypeList>& params) const
		{ return BaseType::DoAddTask(Detail::Binder<FunctorType, typename TypeListTransform<ParamTypeList, RemoveReference>::ValueT>(this->_func, params)); }
	};


	template < typename FunctorType >
	AsyncFunction<FunctorType> MakeAsyncFunction(const ITaskExecutorPtr& executor, const FunctorType& func, const FutureExecutionTester& tester = null)
	{ return AsyncFunction<FunctorType>(executor, func, tester); }

	/** @} */

}

#endif
