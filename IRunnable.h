#ifndef __GS_DVRLIB_TOOLKIT_IRUNNABLE_H__
#define __GS_DVRLIB_TOOLKIT_IRUNNABLE_H__


#include <stingray/toolkit/shared_ptr.h>


namespace stingray
{


	/*! \cond GS_INTERNAL */

	struct IRunnable
	{
		virtual ~IRunnable() { }

		virtual void Run() = 0;
	};
	TOOLKIT_DECLARE_PTR(IRunnable);



	template < typename FunctorT >
	class FunctorRunnable : public virtual IRunnable
	{
	private:
		FunctorT	_func;

	public:
		FunctorRunnable(const FunctorT& func)
			: _func(func)
		{ }

		virtual void Run() { _func(); }
	};


	template < typename FunctorT >
	IRunnablePtr MakeFunctorRunnable(const FunctorT& func)
	{ return make_shared<FunctorRunnable<FunctorT> >(func); }

	/*! \endcond */

}
	

#endif
