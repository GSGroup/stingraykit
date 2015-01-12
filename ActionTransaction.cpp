#include <stingray/toolkit/ActionTransaction.h>

#include <stingray/toolkit/log/Logger.h>


namespace stingray
{

	ActionTransaction::ActionTransaction()
		: _active(true)
	{ }


	ActionTransaction::~ActionTransaction()
	{
		TRACER;

		if (_active)
			STINGRAYKIT_TRY("Couldn't rollback transaction!", Rollback());
	}


	void ActionTransaction::Execute(const ActionFunc& actionFunc, const RollbackFunc& rollbackFunc)
	{
		TRACER;

		STINGRAYKIT_CHECK(_active, "Transaction is inactive!");

		actionFunc();

		_rollbackSequence.push_back(rollbackFunc);
	}


	void ActionTransaction::Commit()
	{
		TRACER;

		STINGRAYKIT_CHECK(_active, "Transaction is inactive!");

		_active = false;
	}


	void ActionTransaction::Rollback()
	{
		TRACER;

		STINGRAYKIT_CHECK(_active, "Transaction is inactive!");

		for (RollbackSequence::reverse_iterator it = _rollbackSequence.rbegin(); it != _rollbackSequence.rend(); ++it)
			(*it)();

		_active = false;
	}

}
