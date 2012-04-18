#ifndef __GS_STINGRAY_TOOLKIT_ASYNCACTION_H__
#define __GS_STINGRAY_TOOLKIT_ASYNCACTION_H__


#include <stingray/toolkit/signals.h>
#include <stingray/toolkit/optional.h>


namespace stingray
{

	class AsyncAction
	{
		TOOLKIT_NONCOPYABLE(AsyncAction);

		typedef function<void ()>						ActionType;
		typedef function<void (const std::exception&)>	ExceptionHandlerType;

	private:
		ActionType				_action;
		ExceptionHandlerType	_exceptionHandler;
		optional<bool>			_result;
		ThreadPtr				_worker;

	public:
		AsyncAction(const std::string& actionName, const ActionType& action, const ExceptionHandlerType& exceptionHandler = &LogException);
		~AsyncAction();

		signal<void ()> OnSuccess;
		signal<void ()>	OnFailure;

	private:
		void ThreadFunc();

		void OnSuccessPopulator(const function<void ()>& slot);
		void OnFailurePopulator(const function<void ()>& slot);
	};
	TOOLKIT_DECLARE_PTR(AsyncAction);

}


#endif
