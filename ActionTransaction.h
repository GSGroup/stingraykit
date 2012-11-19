#ifndef STINGRAY_TOOLKIT_ACTIONTRANSACTION_H
#define STINGRAY_TOOLKIT_ACTIONTRANSACTION_H


#include <vector>

#include <stingray/toolkit/function.h>


namespace stingray
{

	class ActionTransaction
	{
		TOOLKIT_NONCOPYABLE(ActionTransaction);

		typedef function<void ()> ActionFunc;
		typedef function<void ()> RollbackFunc;

		typedef std::vector<RollbackFunc> RollbackSequence;

	private:
		RollbackSequence	_rollbackSequence;
		bool				_active;

	public:
		ActionTransaction();
		~ActionTransaction();

		void Execute(const ActionFunc& actionFunc, const RollbackFunc& rollbackFunc);

		void Commit();
		void Rollback();
	};

}


#endif
