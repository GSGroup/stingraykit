#ifndef __GS_STINGRAY_TOOLKIT_IASYNCACTION_H__
#define __GS_STINGRAY_TOOLKIT_IASYNCACTION_H__


#include <stingray/toolkit/signals.h>


namespace stingray
{

	struct IAsyncAction
	{
		IAsyncAction()
			: OnSuccess(bind(&IAsyncAction::OnSuccessPopulator, this, _1)),
			  OnFailure(bind(&IAsyncAction::OnFailurePopulator, this, _1))
		{ }

		virtual ~IAsyncAction() { }

		signal<void ()> OnSuccess;
		signal<void ()>	OnFailure;

	protected:
		virtual void OnSuccessPopulator(const function<void ()>& slot) = 0;
		virtual void OnFailurePopulator(const function<void ()>& slot) = 0;
	};
	TOOLKIT_DECLARE_PTR(IAsyncAction);

}


#endif
