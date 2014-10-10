#include <stingray/toolkit/Factory.h>


namespace stingray
{

	FactoryContext::FactoryContext() : _overridingAllowed(false) { }
	FactoryContext::FactoryContext(const ClassRegistry& registry)
		: _registry(registry), _overridingAllowed(true)
	{ }


	FactoryContext::~FactoryContext()
	{
		STINGRAY_TRY("Clean failed",
			MutexLock l(_registryGuard);
			for (ClassRegistry::iterator i = _registry.begin(); i != _registry.end(); ++i)
				delete i->second;
		);
	}


	FactoryContextPtr FactoryContext::Clone() const
	{
		MutexLock l(_registryGuard);
		FactoryContextPtr clone(new FactoryContext(_registry));
		return clone;
	}


	void FactoryContext::Register(const std::string& name, IFactoryObjectCreator* creator)
	{
		Logger::Debug() << "Registering " << name;

		MutexLock l(_registryGuard);

		TOOLKIT_CHECK(_overridingAllowed || _registry.find(name) == _registry.end(), "Class '" + name + "' was already registered!");
		_registry[name] = creator;
	}


	IFactoryObject* FactoryContext::Create(const std::string& name)
	{
		MutexLock l(_registryGuard);

		ClassRegistry::const_iterator it = _registry.find(name);
		TOOLKIT_CHECK(it != _registry.end(), "Class '" + name + "' was not registered!");

		const IFactoryObjectCreator& creator = *it->second;
		return creator.Create();
	}

}

