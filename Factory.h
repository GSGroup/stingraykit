#ifndef STINGRAY_TOOLKIT_FACTORY_H
#define STINGRAY_TOOLKIT_FACTORY_H


#include <stingray/toolkit/Singleton.h>
#include <stingray/toolkit/Version.h>


namespace stingray
{

	struct IFactoryObjectCreator
	{
		virtual ~IFactoryObjectCreator() { }

		virtual IFactoryObject* Create() const = 0;
	};


	class Factory;

	namespace Detail
	{

		template < typename ClassType >
		class SimpleFactoryObjectCreator : public virtual IFactoryObjectCreator
		{
		public:
			virtual IFactoryObject* Create() const { return new ClassType; }
		};


		class FactoryContext
		{
			typedef std::map<std::string, IFactoryObjectCreator*> ClassRegistry;

		private:
			ClassRegistry	_registry;
			Mutex			_registryGuard;

		public:
			FactoryContext() { }
			~FactoryContext();

			template < typename ClassType >
			void Register(const std::string& name)
			{
				unique_ptr<IFactoryObjectCreator> creator(new SimpleFactoryObjectCreator<ClassType>());
				Register(name, creator.get());
				creator.release();
			}

			template < typename ClassType >
			ClassType* Create(const std::string& name)
			{
				unique_ptr<IFactoryObject> factory_obj(Create(name));
				ClassType* object = TOOLKIT_CHECKED_DYNAMIC_CASTER(factory_obj.get());
				factory_obj.release();
				return object;
			}

		private:
			void Register(const std::string& name, IFactoryObjectCreator* creator);

			IFactoryObject* Create(const std::string& name);
		};
		TOOLKIT_DECLARE_PTR(FactoryContext);


		class Factory : public Singleton<Factory>
		{
			TOOLKIT_SINGLETON(Factory);

			friend class stingray::Factory;

		private:
			FactoryContextPtr		_defaultContext;
			FactoryContextWeakPtr	_overriddenContext;

			Mutex					_guard;

		public:
			Factory();

			FactoryContextPtr OverrideContext();

			template < typename ClassType >
			void Register(const std::string& name)
			{
				MutexLock l(_guard);

				const FactoryContextPtr overridden = _overriddenContext.lock();
				if (overridden)
					overridden->Register<ClassType>(name);
				else
					_defaultContext->Register<ClassType>(name);
			}

			template < typename ClassType >
			ClassType* Create(const std::string& name)
			{
				MutexLock l(_guard);

				const FactoryContextPtr overridden = _overriddenContext.lock();
				if (overridden)
					return overridden->Create<ClassType>(name);

				return _defaultContext->Create<ClassType>(name);
			}

		private:
			// Defined in stingray/toolkit/_FactoryClasses.cpp
			void RegisterTypes();
		};

	} //Detail

	class Factory : public Singleton<Factory>
	{
		TOOLKIT_SINGLETON(Factory);

		Factory() { Detail::Factory::Instance().RegisterTypes(); }

	public:
		template < typename ClassType >
		ClassType* Create(const std::string& name)
		{ return Detail::Factory::Instance().Create<ClassType>(name); }
	};

}


#define TOOLKIT_REGISTER_CLASS(Class_) \
	friend class stingray::Detail::SimpleFactoryObjectCreator<Class_>; \
	virtual std::string GetClassName() const { return RemovePrefix(TypeInfo(typeid(Class_)).GetName(), "stingray::"); }


#define TOOLKIT_REGISTER_CLASS_EXPLICIT_IMPL(Class_, ClassName_) stingray::Detail::Factory::Instance().Register<Class_>(ClassName_)

#define TOOLKIT_REGISTER_CLASS_EXPLICIT_0(Class_, ...) TOOLKIT_REGISTER_CLASS_EXPLICIT_IMPL(Class_, #Class_)
#define TOOLKIT_REGISTER_CLASS_EXPLICIT_1(Class_, ClassName_) TOOLKIT_REGISTER_CLASS_EXPLICIT_IMPL(Class_, ClassName_)

#define TOOLKIT_REGISTER_CLASS_EXPLICIT(Class_, ...) TOOLKIT_CAT(TOOLKIT_REGISTER_CLASS_EXPLICIT_, TOOLKIT_NARGS(__VA_ARGS__))(Class_, __VA_ARGS__)


#define TOOLKIT_REGISTER_MIGRATION_CLASS(Class_, ClassName_) \
	friend class stingray::Detail::SimpleFactoryObjectCreator<Class_>; \
	virtual std::string GetClassName() const { return ClassName_; }


#endif
