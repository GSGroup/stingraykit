#ifndef DVRLIB_TOOLKIT_FACTORY_H
#define DVRLIB_TOOLKIT_FACTORY_H

#include <map>
#include <memory>
#include <string>
#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/Singleton.h>
#include <stingray/toolkit/Macro.h>
#include <stingray/toolkit/toolkit.h>
#include <stingray/threads/Thread.h>

/*! \cond GS_INTERNAL */

namespace stingray {

	struct IFactoryObjectCreator {
		virtual std::string Name() const = 0;
		virtual IFactoryObject * Create() const = 0;

		virtual ~IFactoryObjectCreator() {}
	};

	namespace Detail {

		template<typename ClassType>
		class SimpleFactoryObjectCreator : public IFactoryObjectCreator
		{
			std::string TypeName;

		public:
			SimpleFactoryObjectCreator(const std::string &typeName) : TypeName(typeName) {}

			virtual std::string Name() const		{ return TypeName; }
			virtual IFactoryObject *Create() const	{ return new ClassType; }
		};

		class Factory : public Singleton<Factory> {
			TOOLKIT_SINGLETON(Factory);

			Factory() {}

			void Clean()
			{
				MutexLock l(_registrarLock);
				for (RegistrarMap::iterator i = _registrars.begin(); i != _registrars.end(); ++i)
					delete i->second;
			}

		public:

			~Factory() { DVRLIB_TRY_DO_NO_LOGGER("Factory::Clean failed", Clean()); }

			template<typename Type>
			Type *Create(const std::string &name)
			{
				MutexLock l(_registrarLock);
				RegistrarMap::const_iterator i = _registrars.find(name);
				if (i == _registrars.end())
					TOOLKIT_THROW(std::runtime_error("class " + name + " was not registered"));
				const IFactoryObjectCreator& creator = *i->second;
				const std::type_info & type_id = typeid (Type);
				std::auto_ptr<IFactoryObject> factory_obj(creator.Create());
				Type *object = dynamic_cast<Type *> (factory_obj.get());
				if (!object)
					TOOLKIT_THROW(std::runtime_error("Cannot cast " + name + " " + typeid (Type).name()));
				factory_obj.release();
				return object;
			}

			template<typename Type>
			const IFactoryObjectCreator& GetCreator() {
				std::string name(typeid (Type).name());
				MutexLock l(_registrarLock);
				RegistrarMap::const_iterator i = _registrarsByTypeId.find(name);
				if (i == _registrarsByTypeId.end())
					TOOLKIT_THROW(std::runtime_error("class with typeid " + name + " was not registered, check _FactoryClasses.cpp"));
				return *i->second;
			}

			template<typename ClassType>
			void Register(const std::string &name, const std::string &type) {
				MutexLock l(_registrarLock);
				{
					RegistrarMap::const_iterator i = _registrars.find(name);
					if (i != _registrars.end())
						TOOLKIT_THROW(std::runtime_error("class " + name + " was already registered"));
				}
				{
					RegistrarMap::const_iterator i = _registrarsByTypeId.find(type);
					if (i != _registrarsByTypeId.end())
						TOOLKIT_THROW(std::runtime_error("typeid " + name + " was already registered"));
				}
				IFactoryObjectCreator *creator = new SimpleFactoryObjectCreator<ClassType>(name);
				_registrars.insert(RegistrarMap::value_type(name, creator));
				_registrarsByTypeId.insert(RegistrarMap::value_type(type, creator));
			}

			void Dump();
			void RegisterTypes();

		private:
			typedef std::map<const std::string, IFactoryObjectCreator *> RegistrarMap;
			RegistrarMap	_registrars, _registrarsByTypeId;
			Mutex			_registrarLock;
		};

	} //Detail

	class Factory : public Singleton<Factory> {
		TOOLKIT_SINGLETON(Factory);

		Factory() {
			Detail::Factory::Instance().RegisterTypes();
		}

	public:

		template<typename Type>
		static Type *Create(const std::string &name) {
			return Detail::Factory::Instance().Create<Type > (name);
		}
	};

}

#include <stingray/log/Logger.h>
/*! \endcond */

#define TOOLKIT_REGISTER_CLASS(type) \
	friend class stingray::Detail::Factory; \
	friend class stingray::Detail::SimpleFactoryObjectCreator<type>; \
	virtual const IFactoryObjectCreator &GetFactoryObjectCreator() const	{ return GetStaticFactoryObjectCreator(); } \
	static const IFactoryObjectCreator& GetStaticFactoryObjectCreator()		{ return stingray::Detail::Factory::Instance().GetCreator<type>(); }

#define TOOLKIT_REGISTER_CLASS_EXPLICIT(type) stingray::Detail::Factory::Instance().Register<type>(#type, typeid(type).name())

#endif
