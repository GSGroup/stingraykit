#ifndef STINGRAYKIT_FACTORY_H
#define STINGRAYKIT_FACTORY_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/factory/IFactoryObject.h>
#include <stingraykit/Singleton.h>

namespace stingray
{

	namespace Detail
	{

		class Factory;

	}


	class Factory;


	class FactoryContext;
	STINGRAYKIT_DECLARE_PTR(FactoryContext);


	class FactoryContext
	{
		STINGRAYKIT_NONCOPYABLE(FactoryContext);

	public:
		STINGRAYKIT_DECLARE_UNIQ_PTR(IFactoryObject);

		struct IFactoryObjectCreator
		{
			virtual ~IFactoryObjectCreator() { }

			virtual IFactoryObjectUniqPtr Create() const = 0;
		};
		STINGRAYKIT_DECLARE_UNIQ_PTR(IFactoryObjectCreator);

		template < typename ClassType >
		class FactoryObjectCreator final : public virtual IFactoryObjectCreator
		{
			IFactoryObjectUniqPtr Create() const override
			{ return IFactoryObjectUniqPtr(new ClassType()); }
		};

	private:
		using ObjectCreatorsRegistry = std::map<std::string, IFactoryObjectCreatorUniqPtr>;
		using ClassNamesRegistry = std::map<TypeInfo, std::string>;

		friend class Detail::Factory;

	private:
		Mutex						_guard;

		FactoryContextPtr			_baseContext;

		ObjectCreatorsRegistry		_objectCreators;
		ClassNamesRegistry			_classNames;

	public:
		FactoryContext();

		std::string GetClassName(const std::type_info& info) const;

		template < typename ClassType >
		void Register(const std::string& name)
		{ Register(name, TypeInfo(typeid(ClassType)), make_unique_ptr<FactoryObjectCreator<ClassType>>()); }

		template < typename ClassType >
		unique_ptr<ClassType> Create(const std::string& name) const
		{
			IFactoryObjectUniqPtr factoryObj(Create(name));
			ClassType* object = STINGRAYKIT_CHECKED_DYNAMIC_CASTER(STINGRAYKIT_REQUIRE_NOT_NULL(factoryObj.get()));
			factoryObj.release();
			return unique_ptr<ClassType>(object);
		}

	private:
		FactoryContext(const FactoryContextPtr& baseContext);

		void Register(const std::string& name, const TypeInfo& info, IFactoryObjectCreatorUniqPtr&& creator);

		IFactoryObjectUniqPtr Create(const std::string& name) const;
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
				: _rootContext(make_shared_ptr<FactoryContext>())
			{ }

			FactoryContextPtr GetRootContext() const
			{ return _rootContext; }

			FactoryContextPtr InheritRootContext() const
			{ return FactoryContextPtr(new FactoryContext(_rootContext)); }

			template < typename ClassType >
			void Register(const std::string& name)
			{ _rootContext->Register<ClassType>(name);}

			template < typename ClassType >
			unique_ptr<ClassType> Create(const std::string& name) const
			{ return _rootContext->Create<ClassType>(name); }

		private:
			// Defined in stingray/settings/_FactoryClasses.cpp
			void RegisterTypes();
		};

	}


	class Factory : public Singleton<Factory>
	{
		STINGRAYKIT_SINGLETON(Factory);

	private:
		Factory()
		{ Detail::Factory::Instance().RegisterTypes(); }

	public:
		template < typename ClassType >
		unique_ptr<ClassType> Create(const std::string& name) const
		{ return Detail::Factory::Instance().Create<ClassType>(name); }

		FactoryContextPtr GetRootContext() const
		{ return Detail::Factory::Instance().GetRootContext(); }

		FactoryContextPtr InheritRootContext() const
		{ return Detail::Factory::Instance().InheritRootContext(); }

		static std::string RemoveTypeSuffix(const std::string& type, const std::string& suffix);
		static std::string RemoveTypePrefix(const std::string& type, const std::string& prefix);
	};

}


#define STINGRAYKIT_REGISTER_CLASS(Class_) \
		std::string GetClassName() const override \
		{ return stingray::Factory::RemoveTypePrefix(TypeInfo(typeid(Class_)).GetName(), "stingray::"); } \
		friend class stingray::FactoryContext::FactoryObjectCreator<Class_>


#define STINGRAYKIT_REGISTER_CLASS_EXPLICIT_IMPL(Class_, ClassName_) stingray::Detail::Factory::Instance().Register<Class_>(ClassName_)

#define STINGRAYKIT_REGISTER_CLASS_EXPLICIT_0(Class_, ...) STINGRAYKIT_REGISTER_CLASS_EXPLICIT_IMPL(Class_, #Class_)
#define STINGRAYKIT_REGISTER_CLASS_EXPLICIT_1(Class_, ClassName_) STINGRAYKIT_REGISTER_CLASS_EXPLICIT_IMPL(Class_, ClassName_)

#define STINGRAYKIT_REGISTER_CLASS_EXPLICIT(Class_, ...) STINGRAYKIT_CAT(STINGRAYKIT_REGISTER_CLASS_EXPLICIT_, STINGRAYKIT_NARGS(__VA_ARGS__))(Class_, __VA_ARGS__)

#endif
