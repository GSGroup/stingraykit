#ifndef STINGRAYKIT_FUNCTIONTOKEN_H
#define STINGRAYKIT_FUNCTIONTOKEN_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/function/FunctorInvoker.h>
#include <stingraykit/log/Logger.h>
#include <stingraykit/Token.h>

namespace stingray
{

	namespace Detail
	{
		template < typename FuncType >
		class FunctionToken : public virtual IToken
		{
		private:
			FuncType	_cleanupFunc;

		public:
			FunctionToken(const FuncType& cleanupFunc) : _cleanupFunc(cleanupFunc)
			{ }

			FunctionToken(FuncType&& cleanupFunc) : _cleanupFunc(std::move(cleanupFunc))
			{ }

			virtual ~FunctionToken()
			{ STINGRAYKIT_TRY_NO_MESSAGE(FunctorInvoker::InvokeArgs(_cleanupFunc)); }
		};
	}


	template < typename FuncType >
	Token MakeFunctionToken(FuncType&& cleanupFunc)
	{ return MakeToken<Detail::FunctionToken<typename Decay<FuncType>::ValueT>>(std::forward<FuncType>(cleanupFunc)); }

}

#endif
