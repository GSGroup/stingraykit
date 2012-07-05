#ifndef __GS_DVRLIB_TOOLKIT_ONCE_H__
#define __GS_DVRLIB_TOOLKIT_ONCE_H__


#include <stingray/toolkit/function.h>


namespace stingray
{


	class once
	{
		typedef function<void()>	FuncType;

	private:
		mutable shared_ptr<FuncType>	_func;

	public:
		once()
		{ }

		template < typename Callable >
		once(const Callable& func)
			: _func(new FuncType(func))
		{ }

		void operator() () const
		{
			if (_func)
			{
				(*_func)();
				_func.reset();
			}
		}
	};


}


#endif
