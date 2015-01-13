#ifndef STINGRAYKIT_FUNCTIONTOKEN_H
#define STINGRAYKIT_FUNCTIONTOKEN_H


#include <stingray/toolkit/log/Logger.h>
#include <stingray/toolkit/IToken.h>
#include <stingray/toolkit/function/function.h>

namespace stingray
{

	class FunctionToken : public virtual IToken
	{
	private:
		function<void()>	_cleanupFunc;

	public:
		FunctionToken(const function<void()>& cleanupFunc) : _cleanupFunc(cleanupFunc)
		{ }

		virtual ~FunctionToken()
		{ STINGRAYKIT_TRY_NO_MESSAGE(_cleanupFunc()); }
	};

}

#endif
