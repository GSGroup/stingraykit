#ifndef STINGRAY_TOOLKIT_FACTORY_H
#define STINGRAY_TOOLKIT_FACTORY_H

#include <map>
#include <string>

#include <stingray/threads/Thread.h>
#include <stingray/toolkit/Singleton.h>
#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/toolkit.h>
#include <stingray/toolkit/unique_ptr.h>


namespace stingray {

	struct IFactoryObjectCreator {
		virtual std::string Name() const = 0;
		virtual IFactoryObject * Create() const = 0;

		virtual ~IFactoryObjectCreator() {}
	};

	class Factory;

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

		class Factory : public Singleton<Factory>
		{
			TOOLKIT_SINGLETON(Factory);

		private:
			friend class stingray::Factory;

			typedef std::map<const std::string, IFactoryObjectCreator *> RegistrarMap;

			RegistrarMap	_registrars, _registrarsByTypeId;
			Mutex			_registrarLock;

		public:
			template<typename ClassType>
			void Register(const std::string &name, const std::string &type)
			{
				unique_ptr<IFactoryObjectCreator> creator(new SimpleFactoryObjectCreator<ClassType>(name));
				Register(name, type, creator.get());
				creator.release();
			}

			void Dump();

		private:
			Factory() {}
			~Factory() { STINGRAY_TRY("Factory::Clean failed", Clean()); }

			template<typename Type>
			Type *Create(const std::string &name)
			{
				unique_ptr<IFactoryObject> factory_obj(Create(name));
				Type *object = dynamic_cast<Type *> (factory_obj.get());
				if (!object)
					TOOLKIT_THROW(std::runtime_error("Cannot cast " + Demangle(name) + " " + Demangle(typeid(Type).name())));
				factory_obj.release();
				return object;
			}

			template<typename Type>
			const IFactoryObjectCreator& GetCreator()
			{ return GetCreator(typeid (Type).name()); }

			// Defined in stingray/toolkit/_FactoryClasses.cpp
			void RegisterTypes();

			IFactoryObject* Create(const std::string& name);
			const IFactoryObjectCreator& GetCreator(const std::string &name);
			void Register(const std::string &name, const std::string &type, IFactoryObjectCreator *creator);
			void Clean();
		};

	} //Detail

	class Factory : public Singleton<Factory>
	{
		TOOLKIT_SINGLETON(Factory);

		Factory()
		{ Detail::Factory::Instance().RegisterTypes(); }

	public:
		const IFactoryObjectCreator& GetCreator(const std::string& name)
		{ return Detail::Factory::Instance().GetCreator(name); }

		template < typename T >
		T* Create(const std::string& name)
		{ return Detail::Factory::Instance().Create<T>(name); }
	};

}


#define TOOLKIT_REGISTER_CLASS(Class_) \
	friend class stingray::Detail::Factory; \
	friend class stingray::Detail::SimpleFactoryObjectCreator<Class_>; \
	virtual const IFactoryObjectCreator& GetFactoryObjectCreator() const { return stingray::Factory::Instance().GetCreator(typeid(Class_).name()); }

#define TOOLKIT_REGISTER_CLASS_EXPLICIT(Class_) stingray::Detail::Factory::Instance().Register<Class_>(#Class_, typeid(Class_).name())

#endif
