#ifndef __GS_DVRLIB_TOOLKIT_DEPENDSON_H__
#define __GS_DVRLIB_TOOLKIT_DEPENDSON_H__


#include <stingray/toolkit/ServiceProvider.h>


namespace stingray
{

	namespace Detail
	{
		template < typename Dependency >
		struct InitDependency
		{
			static void Call() { ServiceProvider<Dependency>::Get(); }
		};
	}

	/*! \cond GS_INTERNAL */

	template < typename Dependencies >
	class DependsOn
	{
	protected:
		DependsOn()
		{ ForEachInTypeList<typename ToTypeList<Dependencies>::ValueT, Detail::InitDependency>::Do(); }
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
