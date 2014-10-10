#ifndef STINGRAY_TOOLKIT_FACTORY_H
#define STINGRAY_TOOLKIT_FACTORY_H


#include <stingray/toolkit/Singleton.h>


namespace stingray
{

	class Factory;

	class FactoryContext
	{
		struct IFactoryObjectCreator
		{
			virtual ~IFactoryObjectCreator() { }

			virtual IFactoryObject* Create() const = 0;
		};

		template < typename ClassType >
		class FactoryObjectCreator : public virtual IFactoryObjectCreator
		{
			virtual IFactoryObject* Create() const { return new ClassType; }
		};

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
			unique_ptr<IFactoryObjectCreator> creator(new FactoryObjectCreator<ClassType>());
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

	namespace Detail
	{

		class Factory : public Singleton<Factory>
		{
			TOOLKIT_SINGLETON(Factory);

			friend class stingray::Factory;

		private:
			FactoryContextPtr		_rootContext;

		public:
			Factory()
				: _rootContext(new FactoryContext())
			{ }

			FactoryContextPtr GetRootContext() const { return _rootContext; }

			template < typename ClassType >
			void Register(const std::string& name)
			{ _rootContext->Register<ClassType>(name);}

			template < typename ClassType >
			ClassType* Create(const std::string& name)
			{ return _rootContext->Create<ClassType>(name); }

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
		ClassType* Create(const std::string& name) const
		{ return Detail::Factory::Instance().Create<ClassType>(name); }

		FactoryContextPtr GetRootContext() const
		{ return Detail::Factory::Instance().GetRootContext(); }
	};

}


#define TOOLKIT_REGISTER_CLASS(Class_) \
	friend class stingray::FactoryContext::FactoryObjectCreator<Class_>; \
	virtual std::string GetClassName() const { return RemovePrefix(TypeInfo(typeid(Class_)).GetName(), "stingray::"); }


#define TOOLKIT_REGISTER_CLASS_EXPLICIT_IMPL(Class_, ClassName_) stingray::Detail::Factory::Instance().Register<Class_>(ClassName_)

#define TOOLKIT_REGISTER_CLASS_EXPLICIT_0(Class_, ...) TOOLKIT_REGISTER_CLASS_EXPLICIT_IMPL(Class_, #Class_)
#define TOOLKIT_REGISTER_CLASS_EXPLICIT_1(Class_, ClassName_) TOOLKIT_REGISTER_CLASS_EXPLICIT_IMPL(Class_, ClassName_)

#define TOOLKIT_REGISTER_CLASS_EXPLICIT(Class_, ...) TOOLKIT_CAT(TOOLKIT_REGISTER_CLASS_EXPLICIT_, TOOLKIT_NARGS(__VA_ARGS__))(Class_, __VA_ARGS__)


#define TOOLKIT_REGISTER_MIGRATION_CLASS(Class_, ClassName_) \
	friend class stingray::FactoryContext::FactoryObjectCreator<Class_>; \
	virtual std::string GetClassName() const { return ClassName_; }


#endif
