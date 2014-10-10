#include <stingray/toolkit/Factory.h>


namespace stingray
{

	FactoryContext::FactoryContext() : _overridingAllowed(false) { }
	FactoryContext::FactoryContext(const ClassNameToObjectCreatorMap& nameToCreatorMap, const TypeInfoToClassNamesMap& typeToNameMap)
		: _classNameToCreatorMap(nameToCreatorMap), _typeInfoToNameMap(typeToNameMap), _overridingAllowed(true)
	{ }


	FactoryContext::~FactoryContext()
	{
		STINGRAY_TRY("Clean failed",
			MutexLock l(_guard);
			for (ClassNameToObjectCreatorMap::iterator i = _classNameToCreatorMap.begin(); i != _classNameToCreatorMap.end(); ++i)
				delete i->second;
		);
	}


	FactoryContextPtr FactoryContext::Clone() const
	{
		MutexLock l(_guard);
		FactoryContextPtr clone(new FactoryContext(_classNameToCreatorMap, _typeInfoToNameMap));
		return clone;
	}


	std::string FactoryContext::GetClassName(const TypeInfo& typeinfo) const
	{
		MutexLock l(_guard);
		TypeInfoToClassNamesMap::const_iterator it = _typeInfoToNameMap.find(typeinfo);
		return it->second;
	}


	void FactoryContext::Register(const std::string& name, const TypeInfo& typeinfo, IFactoryObjectCreator* creator)
	{
		Logger::Debug() << "Registering " << name;

		MutexLock l(_guard);

		TOOLKIT_CHECK(_overridingAllowed || _classNameToCreatorMap.find(name) == _classNameToCreatorMap.end(), "Class '" + name + "' was already registered!");
		_classNameToCreatorMap[name] = creator;
		_typeInfoToNameMap[typeinfo] = name;
	}


	IFactoryObject* FactoryContext::Create(const std::string& name)
	{
		MutexLock l(_guard);

		ClassNameToObjectCreatorMap::const_iterator it = _classNameToCreatorMap.find(name);
		TOOLKIT_CHECK(it != _classNameToCreatorMap.end(), "Class '" + name + "' was not registered!");

		const IFactoryObjectCreator& creator = *it->second;
		return creator.Create();
	}

}

