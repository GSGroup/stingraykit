#include <stingray/toolkit/Factory.h>


namespace stingray
{

	FactoryContext::~FactoryContext()
	{
		STINGRAY_TRY("Clean failed",
			MutexLock l(_registryGuard);
			for (ClassRegistry::iterator i = _registry.begin(); i != _registry.end(); ++i)
				delete i->second;
		);
	}


	void FactoryContext::Register(const std::string& name, IFactoryObjectCreator* creator)
	{
		Logger::Debug() << "Registering " << name;

		MutexLock l(_registryGuard);

		const ClassRegistry::const_iterator it = _registry.find(name);
		TOOLKIT_CHECK(it == _registry.end(), "Class '" + name + "' was already registered!");

		_registry.insert(std::make_pair(name, creator));
	}


	IFactoryObject* FactoryContext::Create(const std::string& name)
	{
		MutexLock l(_registryGuard);

		ClassRegistry::const_iterator it = _registry.find(name);
		TOOLKIT_CHECK(it != _registry.end(), "Class '" + name + "' was not registered!");

		const IFactoryObjectCreator& creator = *it->second;
		return creator.Create();
	}


	namespace Detail
	{

		Factory::Factory()
			: _defaultContext(new FactoryContext())
		{ }


		FactoryContextPtr Factory::OverrideContext()
		{
			MutexLock l(_guard);

			FactoryContextPtr overridden = _overriddenContext.lock();
			TOOLKIT_CHECK(!overridden, "Context is already overridden!");

			overridden.reset(new FactoryContext());
			_overriddenContext = overridden.weak();

			return overridden;
		}

	}

}

