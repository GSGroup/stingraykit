#ifndef STINGRAY_TOOLKIT_SINGLETON_H
#define STINGRAY_TOOLKIT_SINGLETON_H


#include <stingray/threads/call_once.h>
#include <stingray/toolkit/shared_ptr.h>
#include <stingray/toolkit/toolkit.h>


#define TOOLKIT_SINGLETON(ClassName) \
		friend class ::stingray::Detail::SingletonInstanceHolder<ClassName>; \
		friend class ::stingray::Singleton<ClassName>; \
		TOOLKIT_NONCOPYABLE(ClassName)

#define TOOLKIT_SINGLETON_WITH_TRIVIAL_CONSTRUCTOR(ClassName) \
		TOOLKIT_SINGLETON(ClassName); \
	private: \
		ClassName() { }


namespace stingray
{

	namespace Detail
	{
		template<typename T>
		class SingletonInstanceHolder
		{
			TOOLKIT_NONCOPYABLE(SingletonInstanceHolder);

		private:
			T _instance;

		public:
			SingletonInstanceHolder()	{ }
			~SingletonInstanceHolder()	{ TRACER; }
			T& Get()					{ return _instance; }
		};
	}

	template < typename T >
	class Singleton
	{
		typedef Detail::SingletonInstanceHolder<T> InstanceHolderType;
		TOOLKIT_DECLARE_PTR(InstanceHolderType);

		static void InitInstance()
		{
			InstanceHolderTypePtr ptr(new InstanceHolderType());
			GetInstancePtr() = ptr;
		}

		static InstanceHolderTypePtr& GetInstancePtr()
		{
			static InstanceHolderTypePtr inst;
			return inst;
		}

		static TOOLKIT_DECLARE_ONCE_FLAG(s_initFlag);

	public:
		static T& Instance()
		{
			call_once(s_initFlag, &Singleton::InitInstance);
			return GetInstancePtr()->Get();
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
