#ifndef __GS_DVRLIB_TOOLKIT_SINGLETON_H__
#define __GS_DVRLIB_TOOLKIT_SINGLETON_H__


#include <stingray/toolkit/toolkit.h>


#define TOOLKIT_SINGLETON(ClassName) \
		friend class stingray::Singleton<ClassName>; \
		TOOLKIT_NONCOPYABLE(ClassName)

#define TOOLKIT_SINGLETON_WITH_TRIVIAL_CONSTRUCTOR(ClassName) \
		TOOLKIT_SINGLETON(ClassName); \
	private: \
		ClassName() { }
		

namespace stingray
{

	/** @brief Simple Singleton implementation, NOT THREAD-SAFE!!! */
	template < typename T >
	class Singleton
	{
	public:
		static T& Instance() 
		{
			static T inst;
			return inst;
		}

		static const T& ConstInstance()
		{ return const_cast<const T&>(Instance()); }
	};
	

	template < typename T >
	struct IsSingleton
	{
		template < typename U >
		static YesType Test(const Singleton<U>*);
		static NoType Test(...);
		static const bool Value = sizeof(Test((T*)0)) == sizeof(YesType);
	};

}


#endif
