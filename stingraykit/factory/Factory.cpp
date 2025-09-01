// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/factory/Factory.h>
#include <stingraykit/string/StringUtils.h>

namespace stingray
{

	namespace
	{

		const NamedLogger FactoryLogger("Factory");

	}


	string_view FactoryUtils::RemoveTypePrefix(string_view type, string_view prefix)
	{ return RemovePrefix(type, prefix); }


	string_view FactoryUtils::RemoveTypeSuffix(string_view type, string_view suffix)
	{ return RemoveSuffix(type, suffix); }


	FactoryContext::FactoryContext()
	{ }


	FactoryContext::FactoryContext(const FactoryContextConstPtr& baseContext)
		: _baseContext(STINGRAYKIT_REQUIRE_NOT_NULL(baseContext))
	{ }


	std::string FactoryContext::GetClassName(const std::type_info& info) const
	{
		const TypeInfo info_(info);

		{
			MutexLock l(_guard);

			const ClassNamesRegistry::const_iterator it = _classNames.find(info_);
			if (it != _classNames.end())
				return it->second;
		}

		STINGRAYKIT_CHECK(_baseContext, StringBuilder() % "Class '" % info_.GetClassName() % "' isn't registered");
		return _baseContext->GetClassName(info);
	}


	void FactoryContext::Register(const std::string& name, const TypeInfo& info, Detail::IFactoryObjectCreatorUniqPtr&& creator)
	{
		FactoryLogger.Debug() << "Registering " << name;

		MutexLock l(_guard);

		STINGRAYKIT_CHECK(_objectCreators.find(name) == _objectCreators.end(), StringBuilder() % "Class '" % name % "' is already registered");
		_objectCreators.emplace(name, std::move(creator));

		const ClassNamesRegistry::const_iterator it = _classNames.find(info);
		if (it == _classNames.end())
			_classNames.emplace(info, name);
		else
			FactoryLogger.Warning() << "Ignore name " << name << " for class '" << info.GetClassName() << "', already have name " << it->second;
	}


	Detail::IFactoryObjectUniqPtr FactoryContext::Create(const std::string& name) const
	{
		{
			MutexLock l(_guard);

			const ObjectCreatorsRegistry::const_iterator it = _objectCreators.find(name);
			if (it != _objectCreators.end())
				return it->second->Create();
		}

		STINGRAYKIT_CHECK(_baseContext, StringBuilder() % "Class '" % name % "' isn't registered");
		return _baseContext->Create(name);
	}

}

