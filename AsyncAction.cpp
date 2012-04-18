#include <stingray/toolkit/AsyncAction.h>


namespace stingray
{

	AsyncAction::AsyncAction(const std::string& actionName, const ActionType& action, const ExceptionHandlerType& exceptionHandler)
		: _action(action), _exceptionHandler(exceptionHandler)
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

			signal_locker l1(OnSuccess), l2(OnFailure);
			_result.reset(true);
			OnSuccess();
		}
		catch (const std::exception& ex)
		{
			_exceptionHandler(ex);

			signal_locker l1(OnSuccess), l2(OnFailure);
			_result.reset(false);
			OnFailure();
		}
	}


	void AsyncAction::OnSuccessPopulator(const function<void ()>& slot)
	{
		if (_result && _result.get())
			slot();
	}


	void AsyncAction::OnFailurePopulator(const function<void ()>& slot)
	{
		if (_result && !_result.get())
			slot();
	}

}
