#include <stingray/toolkit/Factory.h>
#include <stingray/log/Logger.h>

/*! \cond GS_INTERNAL */

namespace stingray { namespace Detail
{

	void Factory::Dump()
	{
		Logger::Info() << "Registered " << _registrars.size() << " classes:";
		for(RegistrarMap::const_iterator i = _registrars.begin(); i != _registrars.end(); ++i)
		{
			Logger::Info() << i->first;
		}

		Logger::Info() << "Registered " << _registrarsByTypeId.size() << " type ids:";
		for(RegistrarMap::const_iterator i = _registrarsByTypeId.begin(); i != _registrarsByTypeId.end(); ++i)
		{
			Logger::Info() << i->first;
		}
	}


	void Factory::Clean()
	{
		MutexLock l(_registrarLock);
		for (RegistrarMap::iterator i = _registrars.begin(); i != _registrars.end(); ++i)
			delete i->second;
	}


	IFactoryObject *Factory::Create(const std::string &name)
	{
		MutexLock l(_registrarLock);
		RegistrarMap::const_iterator i = _registrars.find(name);
		if (i == _registrars.end())
			TOOLKIT_THROW(std::runtime_error("class " + name + " was not registered"));
		const IFactoryObjectCreator& creator = *i->second;
		return creator.Create();
	}


	const IFactoryObjectCreator& Factory::GetCreator(const std::string& name)
	{
		MutexLock l(_registrarLock);
		RegistrarMap::const_iterator i = _registrarsByTypeId.find(name);
		if (i == _registrarsByTypeId.end())
			TOOLKIT_THROW(std::runtime_error("class with typeid " + Demangle(name) + " was not registered, check _FactoryClasses.cpp"));
		return *i->second;
	}


	void Factory::Register(const std::string &name, const std::string &type, IFactoryObjectCreator *creator)
	{
		Logger::Debug() << "Registered " << name << " as " << type;
		MutexLock l(_registrarLock);
		{
			RegistrarMap::const_iterator i = _registrars.find(name);
			if (i != _registrars.end())
				TOOLKIT_THROW(std::runtime_error("class " + name + " was already registered"));
		}
		{
			RegistrarMap::const_iterator i = _registrarsByTypeId.find(type);
			if (i != _registrarsByTypeId.end())
				TOOLKIT_THROW(std::runtime_error("typeid " + Demangle(name) + " was already registered"));
		}
		_registrars.insert(RegistrarMap::value_type(name, creator));
		_registrarsByTypeId.insert(RegistrarMap::value_type(type, creator));
	}

}}

/*! \endcond */
