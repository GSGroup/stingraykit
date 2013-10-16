#ifndef STINGRAY_TOOLKIT_DEPENDSON_H
#define STINGRAY_TOOLKIT_DEPENDSON_H


#include <stingray/toolkit/ServiceProvider.h>
#include <stingray/toolkit/Singleton.h>


namespace stingray
{

	template < typename DependencyInitInvoker_ >
	struct CustomDependency
	{
		typedef DependencyInitInvoker_		DependencyInitInvoker;
	};

	namespace Detail
	{
		template < typename T >
		struct IsCustomDependency
		{
			template < typename U >
			static YesType Test(const CustomDependency<U>*);
			static NoType Test(...);
			static const bool Value = sizeof(Test((T*)0)) == sizeof(YesType);
		};

		template < typename Dependency,
			bool IsSingleton_ = IsSingleton<Dependency>::Value,
			bool IsCustomDependecy_ = IsCustomDependency<Dependency>::Value >
		struct InitDependencyImpl
		{
			static void Call() { ServiceProvider<Dependency>::Get(); }
		};

		template < typename Dependency >
		struct InitDependencyImpl<Dependency, true, false>
		{
			static void Call() { Singleton<Dependency>::ConstInstance(); }
		};

		template < typename Dependency >
		struct InitDependencyImpl<Dependency, false, true>
		{
			static void Call() { Dependency::DependencyInitInvoker::Invoke(); }
		};

		template < typename Dependency >
		struct InitDependency : public InitDependencyImpl<Dependency> { };

		template < typename Dependency,
			bool IsSingleton_ = IsSingleton<Dependency>::Value >
		struct AssertDependencyImpl
		{
			static void Call()
			{
				if (!ServiceProvider<Dependency>::IsAlive())
					TOOLKIT_FATAL("Service '" + Demangle(typeid(Dependency).name()) + "' has not been created!");
			}
		};

		template < typename Dependency >
		struct AssertDependencyImpl<Dependency, true>
		{
			static void Call() { Singleton<Dependency>::AssertInstanceCreated(); }
		};

		template < typename Dependency >
		struct AssertDependency : public AssertDependencyImpl<Dependency> { };
	}

	/*! \cond GS_INTERNAL */

	template < typename Dependencies_ >
	class DependsOn
	{
	public:
		typedef typename ToTypeList<Dependencies_>::ValueT	Dependencies;

	protected:
		DependsOn()
		{ ForEachInTypeList<Dependencies, Detail::InitDependency>::Do(); }
	};


	template < typename Dependencies_ >
	class AssertDependenciesAlive
	{
	public:
		typedef typename ToTypeList<Dependencies_>::ValueT	Dependencies;

	protected:
		AssertDependenciesAlive()
		{ ForEachInTypeList<Dependencies, Detail::AssertDependency>::Do(); }
	};


	struct IDependent
	{
		virtual ~IDependent() { }

		virtual void InitDependencies() const = 0;
	};


	template < typename Dependencies >
	class Dependent : public virtual IDependent
	{
	protected:
		Dependent()
		{ }

	public:
		virtual void InitDependencies() const
		{ ForEachInTypeList<typename ToTypeList<Dependencies>::ValueT, Detail::InitDependency>::Do(); }
	};


	template < typename Base, typename Dependencies >
	class DependentWithBase : public Base
	{
	protected:
		DependentWithBase() { }

		template < typename T1 >
		DependentWithBase(const T1& p1) : Base(p1) { }

		template < typename T1, typename T2 >
		DependentWithBase(const T1& p1, const T2& p2) : Base(p1, p2) { }

	public:
		virtual void InitDependencies() const
		{
			CompileTimeAssert<Inherits<Base, IDependent>::Value > ERROR__base_class_must_inherit_from_IDependent;
			(void)ERROR__base_class_must_inherit_from_IDependent;
			Base::InitDependencies(); ForEachInTypeList<typename ToTypeList<Dependencies>::ValueT, Detail::InitDependency>::Do();
		}
	};

	/*! \endcond */

}


#endif
