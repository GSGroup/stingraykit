#ifndef DVRLIB_TOOLKIT_FACTORY_H
#define DVRLIB_TOOLKIT_FACTORY_H

#include <map>
#include <memory>
#include <string>
#include <dvrlib/toolkit/exception.h>
#include <dvrlib/toolkit/Singleton.h>
#include <dvrlib/toolkit/Macro.h>
#include <dvrlib/toolkit/toolkit.h>

/*! \cond GS_INTERNAL */

namespace dvrlib
{
	struct FactoryObjectCreator {
		virtual const char *Name() const = 0;
		virtual IFactoryObject *Create() const = 0;
		virtual ~FactoryObjectCreator() {}
	};

	namespace Detail
	{
		template<typename ClassType>
		class ClassRegistrar : public FactoryObjectCreator
		{
		public:
			virtual const char *Name() const { return TypeName; }
			virtual IFactoryObject *Create() const { return new ClassType; }
			ClassRegistrar(const char *typeName);
		private:
			const char *TypeName;
		};

		class Factory : public Singleton<Factory> {
			TOOLKIT_SINGLETON(Factory);
			Factory() {}
		public:
			template<typename Type>
			Type *Create(const std::string &name)
			{
				RegistrarMap::const_iterator i = _registrars.find(name);
				if (i == _registrars.end())
					TOOLKIT_THROW(std::runtime_error("class " + name + " was not registered"));
				const FactoryObjectCreator& creator = *i->second;
				const std::type_info & type_id = typeid(Type);
				std::auto_ptr<IFactoryObject> factory_obj(creator.Create());
				Type *object = dynamic_cast<Type *>(factory_obj.get());
				if (!object)
					TOOLKIT_THROW(std::runtime_error("Cannot cast " + name + " " + typeid(Type).name()));
				factory_obj.release();
				return object;
			}

			void Register(FactoryObjectCreator *creator)
			{
				std::string name(creator->Name());
				RegistrarMap::const_iterator i = _registrars.find(name);
				if (i != _registrars.end())
					TOOLKIT_THROW(std::runtime_error("class " + name + " was already registered"));
				_registrars.insert(RegistrarMap::value_type(name, creator));
			}

			void Dump();
			void RegisterTypes();

		private:
			typedef std::map<const std::string, FactoryObjectCreator *> RegistrarMap;
			RegistrarMap _registrars;
		};

		template<typename T>
		ClassRegistrar<T>::ClassRegistrar(const char *typeName): TypeName(typeName) {
			Factory::Instance().Register(this);
		}
	} //Detail

	class Factory : public Singleton<Factory> {
		TOOLKIT_SINGLETON(Factory);
		Factory() { Detail::Factory::Instance().RegisterTypes(); }

	public:
		template<typename Type>
		static Type *Create(const std::string &name)
		{  return Detail::Factory::Instance().Create<Type>(name); }
	};

}

#include <dvrlib/log/Logger.h>
/*! \endcond */

#define TOOLKIT_REGISTER_CLASS(type) \
	friend class dvrlib::Detail::Factory; \
	friend class dvrlib::Detail::ClassRegistrar<type>; \
	virtual const FactoryObjectCreator &GetFactoryObjectCreator() const { return GetStaticFactoryObjectCreator(); } \
	static const FactoryObjectCreator& GetStaticFactoryObjectCreator() { static dvrlib::Detail::ClassRegistrar<type> instance(#type); return instance; }

#define TOOLKIT_REGISTER_CLASS_EXPLICIT(type) type::GetStaticFactoryObjectCreator();

#endif
