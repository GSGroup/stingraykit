#ifndef STINGRAY_TOOLKIT_FACTORY_H
#define STINGRAY_TOOLKIT_FACTORY_H


#include <stingray/toolkit/Singleton.h>


namespace stingray
{

	struct IFactoryObjectCreator
	{
		virtual ~IFactoryObjectCreator() { }

		virtual std::string Name() const = 0;

		virtual IFactoryObject* Create() const = 0;
	};


	class Factory;

	namespace Detail
	{

		template < typename ClassType >
		class SimpleFactoryObjectCreator : public virtual IFactoryObjectCreator
		{
		private:
			std::string	_typeName;

		public:
			explicit SimpleFactoryObjectCreator(const std::string& typeName)
				: _typeName(typeName)
			{ }

			virtual std::string Name() const { return _typeName; }

			virtual IFactoryObject* Create() const { return new ClassType; }
		};


		class Factory : public Singleton<Factory>
		{
			TOOLKIT_SINGLETON_WITH_TRIVIAL_CONSTRUCTOR(Factory);

			typedef std::map<std::string, IFactoryObjectCreator*> ClassRegistry;

			friend class stingray::Factory;

		private:
			ClassRegistry	_registry;
			Mutex			_registryGuard;

		public:
			template < typename ClassType >
			void Register(const std::string& name)
			{
				unique_ptr<IFactoryObjectCreator> creator(new SimpleFactoryObjectCreator<ClassType>(name));
				Register(name, creator.get());
				creator.release();
			}

		private:
			~Factory();

			// Defined in stingray/toolkit/_FactoryClasses.cpp
			void RegisterTypes();

			void Register(const std::string& name, IFactoryObjectCreator* creator);

			template < typename T >
			T* Create(const std::string& name)
			{
				unique_ptr<IFactoryObject> factory_obj(Create(name));
				T* object = TOOLKIT_CHECKED_DYNAMIC_CASTER(factory_obj.get());
				factory_obj.release();
				return object;
			}

			IFactoryObject* Create(const std::string& name);

			const IFactoryObjectCreator& GetCreator(const std::string& name);
		};

	} //Detail

	class Factory : public Singleton<Factory>
	{
		TOOLKIT_SINGLETON(Factory);

		Factory() { Detail::Factory::Instance().RegisterTypes(); }

	public:
		const IFactoryObjectCreator& GetCreator(const std::string& name)
		{ return Detail::Factory::Instance().GetCreator(name); }

		template < typename T >
		T* Create(const std::string& name)
		{ return Detail::Factory::Instance().Create<T>(name); }
	};

	template < typename T >
	std::string ExtractClassName()
	{
		const TypeInfo info(typeid(T));
		return RemovePrefix(info.GetName(), "stingray::");
	}

}


#define TOOLKIT_REGISTER_CLASS(Class_) \
	friend class stingray::Detail::Factory; \
	friend class stingray::Detail::SimpleFactoryObjectCreator<Class_>; \
	virtual const IFactoryObjectCreator& GetFactoryObjectCreator() const { return stingray::Factory::Instance().GetCreator(ExtractClassName<Class_>()); }

#define TOOLKIT_REGISTER_CLASS_EXPLICIT(Class_) stingray::Detail::Factory::Instance().Register<Class_>(#Class_)

#endif
