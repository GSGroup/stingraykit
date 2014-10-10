#include <stingray/toolkit/Factory.h>


namespace stingray
{

	FactoryContext::FactoryContext() { }
	FactoryContext::FactoryContext(const FactoryContextPtr& baseContext)
			: _baseContext(baseContext)
		{ }


	FactoryContext::~FactoryContext()
	{
		STINGRAY_TRY("Clean failed",
			MutexLock l(_guard);
			for (ObjectCreatorsRegistry::iterator i = _objectCreators.begin(); i != _objectCreators.end(); ++i)
				delete i->second;
		);
	}


	std::string FactoryContext::GetClassName(const std::type_info& info) const
	{
		const TypeInfo info_(info);

		{
			MutexLock l(_guard);

			ClassNamesRegistry::const_iterator it = _classNames.find(info_);
			if (it != _classNames.end())
				return it->second;
		}

		TOOLKIT_CHECK(_baseContext, "Class '" + info_.GetClassName() + "' isn't registered!");
		return _baseContext->GetClassName(info);
	}


	void FactoryContext::Register(const std::string& name, const TypeInfo& info, IFactoryObjectCreator* creator)
	{
		Logger::Debug() << "Registering " << name;

		MutexLock l(_guard);

		TOOLKIT_CHECK(_objectCreators.find(name) == _objectCreators.end(), "Class '" + name + "' is already registered!");

		_objectCreators[name] = creator;
		_classNames[info] = name;
	}


	IFactoryObject* FactoryContext::Create(const std::string& name)
	{
		{
			MutexLock l(_guard);

			ObjectCreatorsRegistry::const_iterator it = _objectCreators.find(name);
			if (it != _objectCreators.end())
				return it->second->Create();
		}

		TOOLKIT_CHECK(_baseContext, "Class '" + name + "' isn't registered!");
		return _baseContext->Create(name);
	}

}

