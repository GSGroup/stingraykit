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
		template < typename FunctorType >
		class AsyncFunction
			:	public function_info<typename If<IsSame<typename function_info<typename Decay<FunctorType>::ValueT>::RetType, void>::Value,
								typename function_info<typename Decay<FunctorType>::ValueT>::RetType, future<typename function_info<typename Decay<FunctorType>::ValueT>::RetType> >::ValueT,
						typename function_info<typename Decay<FunctorType>::ValueT>::ParamTypes>
		{
			STINGRAYKIT_DEFAULTCOPYABLE(AsyncFunction);
			STINGRAYKIT_DEFAULTMOVABLE(AsyncFunction);

			template < typename RetType_, typename BoundFunctor_ >
			class FuncWrapper : public function_info<void ()>
			{
				STINGRAYKIT_DEFAULTCOPYABLE(FuncWrapper);
				STINGRAYKIT_DEFAULTMOVABLE(FuncWrapper);

			private:
				typename Decay<BoundFunctor_>::ValueT		_func;
				shared_ptr<promise<RetType_> >				_promise;

			public:
				FuncWrapper(BoundFunctor_&& func, const shared_ptr<promise<RetType_> >& promise)
					: _func(std::forward<BoundFunctor_>(func)), _promise(promise)
				{ }

				void operator () () const
				{
					try
					{ _promise->set_value(FunctorInvoker::InvokeArgs(_func)); }
					catch (const std::exception& ex)
					{ _promise->set_exception(MakeExceptionPtr(ex)); }
				}
			};

		private:
			typedef typename Decay<FunctorType>::ValueT				RawFunctorType;
			typedef typename function_info<RawFunctorType>::RetType	RawRetType;

		public:
			typedef typename If<IsSame<RawRetType, void>::Value, RawRetType, future<RawRetType> >::ValueT	RetType;
			typedef typename function_info<RawFunctorType>::ParamTypes										ParamTypes;

		private:
			ITaskExecutorPtr		_executor;
			RawFunctorType			_func;
			FutureExecutionTester	_tester;

		public:
			template < typename ExecutionTester >
			AsyncFunction(const ITaskExecutorPtr& executor, FunctorType&& func, ExecutionTester&& tester)
				: _executor(STINGRAYKIT_REQUIRE_NOT_NULL(executor)), _func(std::forward<FunctorType>(func)), _tester(std::forward<ExecutionTester>(tester))
			{ }

			template < typename... Ts >
			RetType operator () (const Ts&... args) const
			{ return DoAddTask<RawRetType>(Bind(_func, args...)); }

			std::string get_name() const
			{ return "{ AsyncFunction: " + get_function_name(_func) + " }"; }

		private:
			template < typename RetType_, typename BoundFunctor_ >
			typename EnableIf<IsSame<RetType_, void>::Value, RetType_>::ValueT DoAddTask(BoundFunctor_&& func) const
			{ _executor->AddTask(std::forward<BoundFunctor_>(func), _tester); }

			template < typename RetType_, typename BoundFunctor_ >
			typename EnableIf<!IsSame<RetType_, void>::Value, future<RetType_> >::ValueT DoAddTask(BoundFunctor_&& func) const
			{
				typedef promise<RetType_> PromiseType;
				const shared_ptr<PromiseType> promise = make_shared_ptr<PromiseType>();
				_executor->AddTask(FuncWrapper<RetType_, BoundFunctor_>(std::forward<BoundFunctor_>(func), promise), _tester);
				return promise->get_future();
			}
		};
	}


	template < typename FunctorType, typename ExecutionTester = FutureExecutionTester >
	Detail::AsyncFunction<FunctorType> MakeAsyncFunction(const ITaskExecutorPtr& executor, FunctorType&& func, ExecutionTester&& tester = null)
	{ return Detail::AsyncFunction<FunctorType>(executor, std::forward<FunctorType>(func), std::forward<ExecutionTester>(tester)); }

	/** @} */

}

#endif
