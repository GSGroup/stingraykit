#ifndef STINGRAYKIT_FUNCTION_CANCELLABLEFUNCTION_H
#define STINGRAYKIT_FUNCTION_CANCELLABLEFUNCTION_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/TaskLifeToken.h>

namespace stingray
{

	namespace Detail
	{
		template < typename FunctorType >
		class CancellableFunction : public function_info<typename Decay<FunctorType>::ValueT>
		{
			STINGRAYKIT_DEFAULTCOPYABLE(CancellableFunction);
			STINGRAYKIT_DEFAULTMOVABLE(CancellableFunction);

		private:
			typedef typename Decay<FunctorType>::ValueT				RawFunctorType;

		public:
			typedef typename function_info<RawFunctorType>::RetType	RetType;

		private:
			RawFunctorType			_func;
			FutureExecutionTester	_tester;

		public:
			template < typename ExecutionTester >
			CancellableFunction(FunctorType&& func, ExecutionTester&& tester)
				: _func(std::forward<FunctorType>(func)), _tester(std::forward<ExecutionTester>(tester))
			{ }

			template < typename... Ts >
			RetType operator () (Ts&&... args) const
			{ return this->template Do<RetType>(std::forward<Ts>(args)...); }

			std::string get_name() const
			{ return "{ CancellableFunction: " + get_function_name(_func) + " }"; }

		private:
			template < typename RetType_, typename... Ts, typename EnableIf<IsSame<RetType_, void>::Value, int>::ValueT = 0 >
			RetType_ Do(Ts&&... args) const
			{
				LocalExecutionGuard guard(_tester);
				if (guard)
					FunctorInvoker::InvokeArgs(_func, std::forward<Ts>(args)...);
			}

			template < typename RetType_, typename... Ts, typename EnableIf<!IsSame<RetType_, void>::Value, int>::ValueT = 0 >
			RetType_ Do(Ts&&... args) const
			{
				LocalExecutionGuard guard(_tester);
				STINGRAYKIT_CHECK(guard, "Function " + get_function_name(_func) + " was cancelled");

				return FunctorInvoker::InvokeArgs(_func, std::forward<Ts>(args)...);
			}
		};
	}


	template < typename FunctorType, typename ExecutionTester >
	Detail::CancellableFunction<FunctorType> MakeCancellableFunction(FunctorType&& func, ExecutionTester&& tester)
	{ return Detail::CancellableFunction<FunctorType>(std::forward<FunctorType>(func), std::forward<ExecutionTester>(tester)); }

}

#endif
