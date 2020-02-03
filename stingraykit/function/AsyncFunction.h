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

	template < typename FunctorType >
	class AsyncFunction
		:	public function_info<typename If<IsSame<typename function_info<FunctorType>::RetType, void>::Value,
							typename function_info<FunctorType>::RetType, future<typename function_info<FunctorType>::RetType> >::ValueT,
					typename function_info<FunctorType>::ParamTypes>
	{
		STINGRAYKIT_DEFAULTCOPYABLE(AsyncFunction);
		STINGRAYKIT_DEFAULTMOVABLE(AsyncFunction);

	private:
		typedef typename function_info<FunctorType>::RetType	RawRetType;

	public:
		typedef typename If<IsSame<RawRetType, void>::Value, RawRetType, future<RawRetType> >::ValueT	RetType;
		typedef typename function_info<FunctorType>::ParamTypes											ParamTypes;

	private:
		ITaskExecutorPtr		_executor;
		FunctorType				_func;
		FutureExecutionTester	_tester;

	public:
		AsyncFunction(const ITaskExecutorPtr& executor, const FunctorType& func, const FutureExecutionTester& tester)
			: _executor(STINGRAYKIT_REQUIRE_NOT_NULL(executor)), _func(func), _tester(tester)
		{ }

		template < typename... Ts >
		RetType operator () (const Ts&... args) const
		{ return DoAddTask<RawRetType>(Bind(_func, args...)); }

		std::string get_name() const
		{ return "{ AsyncFunction: " + get_function_name(_func) + " }"; }

	private:
		template < typename RetType_ >
		typename EnableIf<IsSame<RetType_, void>::Value, RetType_>::ValueT DoAddTask(const function<RetType_ ()>& func) const
		{ _executor->AddTask(func, _tester); }

		template < typename RetType_ >
		typename EnableIf<!IsSame<RetType_, void>::Value, future<RetType_> >::ValueT DoAddTask(const function<RetType_ ()>& func) const
		{
			typedef promise<RetType_> PromiseType;
			const shared_ptr<PromiseType> promise = make_shared_ptr<PromiseType>();
			_executor->AddTask(Bind(&AsyncFunction::FuncWrapper<RetType_>, func, promise), _tester);
			return promise->get_future();
		}

		template < typename RetType_ >
		static void FuncWrapper(const function<RetType_ ()>& func, const shared_ptr<promise<RetType_> >& promise)
		{
			try
			{ promise->set_value(func()); }
			catch (const std::exception& ex)
			{ promise->set_exception(MakeExceptionPtr(ex)); }
		}
	};


	template < typename FunctorType >
	AsyncFunction<FunctorType> MakeAsyncFunction(const ITaskExecutorPtr& executor, const FunctorType& func, const FutureExecutionTester& tester = null)
	{ return AsyncFunction<FunctorType>(executor, func, tester); }

	/** @} */

}

#endif
