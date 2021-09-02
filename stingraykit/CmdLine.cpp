#include <stingraykit/CmdLine.h>

#include <stingraykit/collection/ArrayList.h>
#include <stingraykit/collection/ForEach.h>

namespace stingray
{

	CmdLine::CmdLine()
		:	_commands(make_shared_ptr<ArrayList<ICommandHandlerPtr>>())
	{ }


	bool CmdLine::Execute(const std::string& cmd)
	{
		if (!cmd.empty() && cmd[0] == '#')
			return true;

		FOR_EACH(const ICommandHandlerPtr handler IN _commands)
			if (handler->Execute(cmd))
				return true;

		return false;
	}


	void CmdLine::Complete(const std::string& cmd, CompletionResults& results) const
	{
		FOR_EACH(const ICommandHandlerPtr handler IN _commands)
			handler->Complete(cmd, results);
	}

}
