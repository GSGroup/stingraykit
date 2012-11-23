#ifndef __GS_DVRLIB_TOOLKIT_SINGLETON_H__
#define __GS_DVRLIB_TOOLKIT_SINGLETON_H__


#include <stingray/threads/call_once.h>
#include <stingray/toolkit/shared_ptr.h>
#include <stingray/toolkit/toolkit.h>


#define TOOLKIT_SINGLETON(ClassName) \
		friend class stingray::shared_ptr<ClassName>; \
		friend class stingray::Singleton<ClassName>; \
		TOOLKIT_NONCOPYABLE(ClassName)

#define TOOLKIT_SINGLETON_WITH_TRIVIAL_CONSTRUCTOR(ClassName) \
		TOOLKIT_SINGLETON(ClassName); \
	private: \
		ClassName() { }


namespace stingray
{

	template < typename T >
	class Singleton
	{
		static void InitInstance()
		{
			shared_ptr<T>& ptr = GetInstancePtr();
			ptr.reset(new T);
		}

		static shared_ptr<T>& GetInstancePtr()
		{
			static shared_ptr<T> inst;
			return inst;
		}

		static TOOLKIT_DECLARE_ONCE_FLAG(s_initFlag);

	public:
		static T& Instance()
		{
			call_once(s_initFlag, &Singleton::InitInstance);
			return *GetInstancePtr();
		}

		static const T& ConstInstance()
		{ return const_cast<const T&>(Instance()); }
	};


	template< typename T >
	TOOLKIT_DEFINE_ONCE_FLAG(Singleton<T>::s_initFlag);


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
