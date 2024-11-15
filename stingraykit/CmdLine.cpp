// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

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

		const size_t key = Enumerable::FirstOrDefault(KeysEnumerable(commands->Reverse())).transform(Bind(std::plus<size_t>(), 1, _1)).get_value_or(0);
		STINGRAYKIT_CHECK(commands->Add(key, handler), LogicException());

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
