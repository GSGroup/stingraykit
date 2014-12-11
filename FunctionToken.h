#ifndef STINGRAY_TOOLKIT_FUNCTIONTOKEN_H
#define STINGRAY_TOOLKIT_FUNCTIONTOKEN_H


#include <stingray/toolkit/log/Logger.h>
#include <stingray/toolkit/IToken.h>
#include <stingray/toolkit/function.h>

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
		{ STINGRAY_TRY_NO_MESSAGE(_cleanupFunc()); }
	};

}

#endif
