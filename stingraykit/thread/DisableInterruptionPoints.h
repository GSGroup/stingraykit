#ifndef STINGRAYKIT_THREAD_DISABLEINTERRUPTIONPOINTS_H
#define STINGRAYKIT_THREAD_DISABLEINTERRUPTIONPOINTS_H

// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/Thread.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_threads
	 * @{
	 */

	template < typename FuncType, typename ModifierToken >
	class ExecutionContextModifier : public function_info<FuncType>
	{
	public:
		typedef typename function_info<FuncType>::RetType 		RetType;
		typedef typename function_info<FuncType>::ParamTypes	ParamTypes;

	private:
		FuncType	_func;

	public:
		ExecutionContextModifier(const FuncType& func) : _func(func)
		{ }

		template < typename... Ts >
		RetType operator () (Ts&&... args) const
		{
			ModifierToken token;
			return _func(std::forward<Ts>(args)...);
		}
	};


	struct DisableInterruptionPointsToken : public EnableInterruptionPoints
	{
		DisableInterruptionPointsToken() : EnableInterruptionPoints(false) { }
	};


	template < typename FuncType >
	ExecutionContextModifier<FuncType, DisableInterruptionPointsToken> DisableInterruptionPoints(const FuncType& func)
	{ return ExecutionContextModifier<FuncType, DisableInterruptionPointsToken>(func); }

	/** @} */

}

#endif
