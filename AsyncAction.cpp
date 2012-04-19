#include <stingray/toolkit/AsyncAction.h>


namespace stingray
{

	AsyncAction::AsyncAction(const std::string& actionName, const ActionType& action, const ExceptionHandlerType& exceptionHandler)
		: _action(action), _exceptionHandler(exceptionHandler), _success(false), _failure(false)
	{ _worker.reset(new Thread(actionName + "Executor", bind(&AsyncAction::ThreadFunc, this))); }


	AsyncAction::~AsyncAction()
	{
		_worker->Interrupt();
		_worker->Join();
	}


	void AsyncAction::ThreadFunc()
	{
		try
		{
			_action();

			signal_locker l(OnSuccess);
			_success = true;
			OnSuccess();
		}
		catch (const std::exception& ex)
		{
			_exceptionHandler(ex);

			signal_locker l(OnFailure);
			_failure = true;
			OnFailure();
		}
	}


	void AsyncAction::OnSuccessPopulator(const function<void ()>& slot)
	{
		if (_success)
			slot();
	}


	void AsyncAction::OnFailurePopulator(const function<void ()>& slot)
	{
		if (_failure)
			slot();
	}

}
