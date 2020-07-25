#ifndef STINGRAYKIT_FUNCTION_CANCELLABLEFUNCTION_H
#define STINGRAYKIT_FUNCTION_CANCELLABLEFUNCTION_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/PerfectForwarding.h>
#include <stingraykit/TaskLifeToken.h>

namespace stingray
{

	template < typename FunctorType >
	class CancellableFunction : public function_info<FunctorType>
	{
	public:
		typedef typename function_info<FunctorType>::RetType		RetType;

	private:
		FunctorType				_func;
		FutureExecutionTester	_tester;

	public:
		CancellableFunction(const FunctorType& func, const FutureExecutionTester& tester)
			: _func(func), _tester(tester)
		{ }

		STINGRAYKIT_PERFECT_FORWARDING(RetType, operator (), Do)

		std::string get_name() const
		{ return "{ CancellableFunction: " + get_function_name(_func) + " }"; }

	private:
		template < typename ParamTypeList_ >
		RetType Do(const Tuple<ParamTypeList_>& params) const
		{ return this->template DoImpl<RetType>(params); }

		template < typename RetType_, typename ParamTypeList_ >
		RetType_ DoImpl(const Tuple<ParamTypeList_>& params, typename EnableIf<IsSame<RetType_, void>::Value, Dummy>::ValueT* = 0) const
		{
			LocalExecutionGuard guard(_tester);
			if (guard)
				FunctorInvoker::Invoke(_func, params);
		}

		template < typename RetType_, typename ParamTypeList_ >
		RetType_ DoImpl(const Tuple<ParamTypeList_>& params, typename EnableIf<!IsSame<RetType_, void>::Value, Dummy>::ValueT* = 0) const
		{
			LocalExecutionGuard guard(_tester);
			STINGRAYKIT_CHECK(guard, "Function " + get_function_name(_func) + " was cancelled");

			return FunctorInvoker::Invoke(_func, params);
		}
	};


	template < typename FunctorType >
	CancellableFunction<FunctorType> MakeCancellableFunction(const FunctorType& func, const FutureExecutionTester& tester)
	{ return CancellableFunction<FunctorType>(func, tester); }

}

#endif
