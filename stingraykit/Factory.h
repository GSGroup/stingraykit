#ifndef STINGRAYKIT_FACTORY_H
#define STINGRAYKIT_FACTORY_H


#include <stingraykit/Singleton.h>


namespace stingray
{

	namespace Detail { class Factory; }

	class Factory;

	class FactoryContext;
	STINGRAYKIT_DECLARE_PTR(FactoryContext);

	class FactoryContext
	{
		STINGRAYKIT_NONCOPYABLE(FactoryContext);

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

		typedef std::map<std::string, IFactoryObjectCreator*>	ObjectCreatorsRegistry;
		typedef std::map<TypeInfo, std::string>					ClassNamesRegistry;

		friend class Detail::Factory;

	private:
		Mutex						_guard;

		FactoryContextPtr			_baseContext;

		ObjectCreatorsRegistry		_objectCreators;
		ClassNamesRegistry			_classNames;

	public:
		FactoryContext();
		~FactoryContext();

		std::string GetClassName(const std::type_info& info) const;

		template < typename ClassType >
		void Register(const std::string& name)
		{
			unique_ptr<IFactoryObjectCreator> creator(new FactoryObjectCreator<ClassType>());
			Register(name, TypeInfo(typeid(ClassType)), creator.get());
			creator.release();
		}

		template < typename ClassType >
		ClassType* Create(const std::string& name)
		{
			unique_ptr<IFactoryObject> factory_obj(Create(name));
			ClassType* object = STINGRAYKIT_CHECKED_DYNAMIC_CASTER(factory_obj.get());
			factory_obj.release();
			return object;
		}

	private:
		FactoryContext(const FactoryContextPtr& baseContext);

		void Register(const std::string& name, const TypeInfo& info, IFactoryObjectCreator* creator);

		IFactoryObject* Create(const std::string& name);
	};

	namespace Detail
	{

		class Factory : public Singleton<Factory>
		{
			STINGRAYKIT_SINGLETON(Factory);

			friend class stingray::Factory;

		private:
			FactoryContextPtr		_rootContext;

		public:
			Factory()
				: _rootContext(new FactoryContext())
			{ }

			FactoryContextPtr GetRootContext() const { return _rootContext; }
			FactoryContextPtr InheritRootContext() const
			{
				const FactoryContextPtr context(new FactoryContext(_rootContext));
				return context;
			}

			template < typename ClassType >
			void Register(const std::string& name)
			{ _rootContext->Register<ClassType>(name);}

			template < typename ClassType >
			ClassType* Create(const std::string& name)
			{ return _rootContext->Create<ClassType>(name); }

		private:
			// Defined in stingray/settings/_FactoryClasses.cpp
			void RegisterTypes();
		};

	} //Detail

	class Factory : public Singleton<Factory>
	{
		STINGRAYKIT_SINGLETON(Factory);

		Factory() { Detail::Factory::Instance().RegisterTypes(); }

	public:
		template < typename ClassType >
		ClassType* Create(const std::string& name) const
		{ return Detail::Factory::Instance().Create<ClassType>(name); }

		FactoryContextPtr GetRootContext() const
		{ return Detail::Factory::Instance().GetRootContext(); }

		FactoryContextPtr InheritRootContext() const
		{ return Detail::Factory::Instance().InheritRootContext(); }
	};

}


#define STINGRAYKIT_REGISTER_CLASS(Class_) \
	friend class stingray::FactoryContext::FactoryObjectCreator<Class_>; \
	virtual std::string GetClassName() const { return RemovePrefix(TypeInfo(typeid(Class_)).GetName(), "stingray::"); }


#define STINGRAYKIT_REGISTER_CLASS_EXPLICIT_IMPL(Class_, ClassName_) stingray::Detail::Factory::Instance().Register<Class_>(ClassName_)

#define STINGRAYKIT_REGISTER_CLASS_EXPLICIT_0(Class_, ...) STINGRAYKIT_REGISTER_CLASS_EXPLICIT_IMPL(Class_, #Class_)
#define STINGRAYKIT_REGISTER_CLASS_EXPLICIT_1(Class_, ClassName_) STINGRAYKIT_REGISTER_CLASS_EXPLICIT_IMPL(Class_, ClassName_)

#define STINGRAYKIT_REGISTER_CLASS_EXPLICIT(Class_, ...) STINGRAYKIT_CAT(STINGRAYKIT_REGISTER_CLASS_EXPLICIT_, STINGRAYKIT_NARGS(__VA_ARGS__))(Class_, __VA_ARGS__)


#define STINGRAYKIT_REGISTER_MIGRATION_CLASS(Class_, ClassName_) \
	friend class stingray::FactoryContext::FactoryObjectCreator<Class_>; \
	virtual std::string GetClassName() const { return ClassName_; }


#endif
