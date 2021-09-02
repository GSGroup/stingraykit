#include <stingraykit/CmdLine.h>

#include <stingraykit/collection/ForEach.h>
#include <stingraykit/collection/KeyValueEnumerableHelpers.h>
#include <stingraykit/collection/MapObservableDictionary.h>
#include <stingraykit/FunctionToken.h>

namespace stingray
{

	CmdLine::CmdLine()
		:	_commands(make_shared_ptr<MapObservableDictionary<size_t, ICommandHandlerPtr>>())
	{ }


	bool CmdLine::Execute(const std::string& cmd)
	{
		if (!cmd.empty() && cmd[0] == '#')
			return true;

		FOR_EACH(const ICommandHandlerPtr handler IN ValuesEnumerable(_commands))
			if (handler->Execute(cmd))
				return true;

		return false;
	}


	void CmdLine::Complete(const std::string& cmd, CompletionResults& results) const
	{
		FOR_EACH(const ICommandHandlerPtr handler IN ValuesEnumerable(_commands))
			handler->Complete(cmd, results);
	}


	Token CmdLine::Register(const CommandsPtr& commands, const ICommandHandlerPtr& handler)
	{
		MutexLock l(commands->GetSyncRoot());

		const size_t key = commands->IsEmpty() ? 0 : Enumerable::First(commands->Reverse()).Key + 1;
		commands->Set(key, handler);

		return MakeFunctionToken(Bind(&CmdLine::Unregister, commands, key));
	}


	void CmdLine::Unregister(const CommandsPtr& commands, size_t key)
	{
		ICommandHandlerPtr handler;

		{
			MutexLock l(commands->GetSyncRoot());
			handler = commands->Get(key);
			commands->Remove(key);
		}

		handler->Release();
	}

}
