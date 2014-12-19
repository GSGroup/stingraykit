#ifndef STINGRAY_TOOLKIT_SINGLETON_H
#define STINGRAY_TOOLKIT_SINGLETON_H


#include <stingray/toolkit/log/Logger.h>
#include <stingray/toolkit/thread/call_once.h>
#include <stingray/toolkit/unique_ptr.h>
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
		class SingletonInstanceHolder : public T
		{
			TOOLKIT_NONCOPYABLE(SingletonInstanceHolder);

		public:
			SingletonInstanceHolder()	{ }
			~SingletonInstanceHolder()	{ TRACER; }
			T& Get()					{ return *this; }
		};
	}

	template < typename T >
	class Singleton
	{
		typedef Detail::SingletonInstanceHolder<T>	InstanceHolderType;
		typedef unique_ptr<InstanceHolderType>		InstanceHolderTypePtr;

		static void InitInstance()
		{
			InstanceHolderTypePtr ptr;
			try
			{ ptr.reset(new InstanceHolderType()); }
			catch(const std::exception& ex)
			{ Logger::Error() << "An exception in " << Demangle(typeid(T).name()) << " singleton constructor: " << diagnostic_information(ex); }
			ptr.swap(GetInstancePtr());
		}

		static void AssertInstance()
		{
			TOOLKIT_FATAL("Singleton '" + Demangle(typeid(T).name()) + "' has not been created!");
		}

		static InstanceHolderTypePtr& GetInstancePtr()
		{
			static InstanceHolderTypePtr inst;
			return inst;
		}

		static TOOLKIT_DECLARE_ONCE_FLAG(s_initFlag);

	public:
		static bool IsAlive() { return GetInstancePtr(); }

		static T& Instance()
		{
			call_once(s_initFlag, &Singleton::InitInstance);
			if (!GetInstancePtr())
				TOOLKIT_THROW("Singleton '" + Demangle(typeid(T).name()) + "' could not be created!");
			return GetInstancePtr()->Get();
		}

		static const T& ConstInstance()
		{ return Instance(); }

		static void AssertInstanceCreated()
		{ call_once(s_initFlag, &Singleton::AssertInstance); }
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
