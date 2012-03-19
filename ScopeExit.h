#ifndef __GS_DVRLIB_TOOLKIT_SCOPEEXIT_H__
#define __GS_DVRLIB_TOOLKIT_SCOPEEXIT_H__


#include <stingray/toolkit/toolkit.h>
#include <stingray/toolkit/function.h>
#include <stingray/toolkit/Macro.h>


/*! \cond GS_INTERNAL */

namespace stingray
{

	class ScopeExitInvoker
	{
	private:
		TOOLKIT_NONCOPYABLE(ScopeExitInvoker);
		function<void()>	_func;

	public:
		ScopeExitInvoker(const function<void()>& func)
			: _func(func)
		{ }

		~ScopeExitInvoker()
		{ _func(); }
	};

#define TOOLKIT_SCOPE_EXIT(...) ScopeExitInvoker TOOLKIT_CAT(scope_exit_invoker_, __LINE__)(__VA_ARGS__)

	
}

/*! \endcond */


#endif
