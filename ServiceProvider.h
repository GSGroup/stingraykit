#ifndef STINGRAY_TOOLKIT_SERVICEPROVIDER_H
#define STINGRAY_TOOLKIT_SERVICEPROVIDER_H

#include <list>
#include <typeinfo>

#include <stingray/threads/Thread.h>
#include <stingray/toolkit/ICreator.h>
#include <stingray/toolkit/MetaProgramming.h>
#include <stingray/toolkit/ServiceTraits.h>
#include <stingray/toolkit/Singleton.h>
#include <stingray/toolkit/SystemProfiler.h>
#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/signals.h>


namespace stingray
{

	/**
	 * @addtogroup core_services
	 * @{
	 */

	/**
	 * @page core_services_overview
	 * @section sec Services overview
	 * This page contains the subsections @ref core_services_overview_using and @ref core_services_overview_creating.
	 *
	 * The services system is the API abstraction layer of stingray.
	 *
	 * A service is a singleton that has an interface and implementation, where the implementation has explicitly declared dependencies.
	 * Service can be created only if all the dependencies have been also created.
	 *
	 * The frontend for using services is the ServiceProvider template. It holds the instance of a services, manages all dependencies, and handles some errors (e.g. cyclic dependencies)
	 *
	 * @subsection core_services_overview_using Using services
	 * Using services is very simple.
	 *
	 * Most often, you just have to call the ServiceProvider::Get method to get access to the service that you need:
	 * @code
	 * AudioOutputProvider::Get().SetVolume(50);
	 * @endcode
	 *
	 * Sometimes you do not want to fail if a service could not be created, but want to do something if it has been created, and has not been destroyed ('weak' dependency). There are two methods of doing that.
	 *
	 * For some one-time stuff, you can try to create a service, and then use ServiceProvider::IsAlive method to check if everything is OK:
	 * @code
	 * try { AudioOutputProvider::Get(); } // Initializing the service
	 * catch (const std::exception& ex) { std::cerr << "Could not initialize AudioOutput: " << ex.what(); }
	 * // ...
	 * if (AudioOutputProvider::IsAlive())
	 *     AudioOutputProvider::Get().Mute(false);
	 * @endcode
	 *
	 * There are also OnServiceCreated and on OnServiceDestroyed signals in ServiceProvider. You should use them if you need longer interaction with a service
	 * @code
	 * void StartMonitoringAudioOutput();
	 * void StoptMonitoringAudioOutput();
	 * // ...
	 * AudioOutputProvider::OnServiceCreated.connect(&StartMonitoringAudioOutput);
	 * AudioOutputProvider::OnServiceDestroyed.connect(&StopMonitoringAudioOutput);
	 * @endcode
	 *
	 * @subsection core_services_overview_creating Creating services
	 * Creating a service is also quite simple.
	 *
	 * If you want to implement an existing service interface, you should create a C++ class that implements this interface and also inherits the ServiceTraits class.
	 * The first parameter of ServiceTraits template is an interface of the service it implements (or a typelist in case of multiple services), and the second is a typelist of dependencies.
	 * @code
	 * namespace stingray {
	 * namespace my_new_platform
	 * {
	 *     class MyAudioOutput : public virtual IAudioOutput, public ServiceTraits<IAudioOutput, TypeList_3<ILogger, ISomeService, ISomeOtherService> >
	 *     {
	 *         // ...
	 *     };
	 * }}
	 * @endcode
	 *
	 * After that you have to add the service creator to AudioOutputProvider.cpp
	 * @code
	 * // ...
	 * #elif PLATFORM_CELESTIAL
	 * #   include <stingray/platform/celestial/output/AudioOutput.h>
	 * #elif PLATFORM_MY_NEW_PLATFORM
	 * #   include <stingray/platform/my_new_platform/output/MyAudioOutput.h>
	 * // ...
	 * #elif PLATFORM_CELESTIAL
	 *     TOOLKIT_DEFINE_SERVICE_CREATOR(celestial::AudioOutput, new DefaultConstructorCreator<IAudioOutput, celestial::AudioOutput>());
	 * #elif PLATFORM_MY_NEW_PLATFORM
	 *     TOOLKIT_DEFINE_SERVICE_CREATOR(my_new_platform::MyAudioOutput, new DefaultConstructorCreator<IAudioOutput, my_new_platform::MyAudioOutput>());
	 * // ...
	 * @endcode
	 *
	 * ---
	 *
	 * If you want to create a new service, you should design an interface for the service:
	 * @code
	 * namespace stingray
	 * {
	 *     struct IMyNewService
	 *     {
	 *         virtual ~IMyNewService() { }
	 *
	 *         int SomeMethod(const std::string& someParameter) = 0;
	 *     };
	 * }
	 * @endcode
	 *
	 * And then create the service provider.
	 *
	 * MyNewServiceProvider.h:
	 * @code
	 * #include <path_to_header_file/IMyNewService.h>
	 *
	 * namespace stingray
	 * {
	 *     typedef ServiceProvider<IMyNewService>    MyNewServiceProvider;
	 * }
	 * @endcode
	 *
	 * MyNewServiceProvider.cpp
	 * @code
	 * #include <path_to_header_file/MyNewServiceProvider.h>
	 *
	 * // .. Some platform-specific ifdefs and includes
	 *
	 * namespace stingray
	 * {
	 * #ifdef PLATFORM_SOMETHING
	 *     // ...
	 *     // Some platform-specific ifdefs and service creators
	 * #else
	 *     // The default implementation, you should use TOOLKIT_SERVICE_NOT_SUPPORTED if there is no default.
	 *     TOOLKIT_DEFINE_SERVICE_CREATOR(DefaultMyNewService, new DefaultConstructorCreator<IMyNewService, DefaultMyNewService>());
	 * #endif
	 * }
	 * @endcode
	 */


	/**
	 * @brief Declares that there is no service implementation. Is used to disable a service and all dependent services on a specific platform.
	 * @param[in] ServiceInterface_ The interface of the service
	 * @par Example:
	 * @code
	 * TOOLKIT_SERVICE_NOT_SUPPORTED(IHDMI);
	 * @endcode
	 */
#define TOOLKIT_SERVICE_NOT_SUPPORTED(ServiceInterface_) \
		template < > \
		stingray::ServiceProvider<ServiceInterface_>::IServiceCreatorPtr	\
			stingray::ServiceProvider<ServiceInterface_>::s_serviceCreator(new stingray::NotSupportedServiceCreator<ServiceInterface_>)

	/**
	 * @brief Defines a service implementation creator
	 * @param[in] Service_ The service implementation class
	 * @param[in] ... A pointer to an ICreator object for the service implementation
	 * @par Example:
	 * @code
	 * TOOLKIT_DEFINE_SERVICE_CREATOR(mstar::HDMI, new DefaultConstructorCreator<IHDMI, HDMIWrapper<mstar::HDMI> >());
	 * @endcode
	 */
#define TOOLKIT_DEFINE_SERVICE_CREATOR(Service_, ...) \
		namespace { stingray::CompileTimeAssert<stingray::GetTypeListLength<Service_::ServiceInterfaces>::Value == 1> PLEASE_USE_DEFINE_SERVICE_CREATOR_EX_IF_YOU_IMPLEMENT_MORE_THAN_ONE_INTERFACE; } \
		template < > \
		stingray::ServiceProvider<Service_::ServiceInterfaces::ValueT>::IServiceCreatorPtr	\
			stingray::ServiceProvider<Service_::ServiceInterfaces::ValueT>::s_serviceCreator(new stingray::ServiceCreator<Service_, Service_::ServiceInterfaces::ValueT>(stingray::shared_ptr<stingray::ICreator<Service_::ServiceInterfaces::ValueT> >(__VA_ARGS__)))

	/**
	 * @brief Gets a pointer to a service implementation creator in runtime. May be used to hook the default implementation of a service.
	 * @param[in] ServiceInterface_ The service interface
	 * @returns A shared_ptr to the IServiceCreator object for the service implementation
	 * @par Example:
	 * @code
	 * TOOLKIT_GET_SERVICE_CREATOR(IHDMI)->InitDependencies(); // Initialize the dependencies
	 * shared_ptr<IHDMI> hdmi = TOOLKIT_GET_SERVICE_CREATOR(IHDMI)->Create(); // Create a service instance
	 * @endcode
	 */
#define TOOLKIT_GET_SERVICE_CREATOR(ServiceInterface_) \
		stingray::ServiceProvider<ServiceInterface_>::GetCreator()

	/**
	 * @brief Sets a pointer to a service implementation creator in runtime. Must be called before the first ServiceProvider<ServiceInterface_>::Get() invokation.
	 * @param[in] Service_ The service implementation class
	 * @param[in] ... A pointer to an ICreator object for the service implementation
	 * @par Example:
	 * @code
	 * // Setting the IUIServices implementation from the application that uses the stingray core library
	 * TOOLKIT_SET_SERVICE_CREATOR(argile::UIServices, new DefaultConstructorCreator<IUIServices, argile::UIServices>());
	 * @endcode
	 */
#define TOOLKIT_SET_SERVICE_CREATOR(Service_, ...) \
		stingray::ServiceProvider<Service_::ServiceInterfaces::ValueT>::SetCreator(stingray::make_shared<stingray::ServiceCreator<Service_, Service_::ServiceInterfaces::ValueT> >(stingray::shared_ptr<stingray::ICreator<Service_::ServiceInterfaces::ValueT> >(__VA_ARGS__)))

	/**
	 * @brief Defines a service implementation creator for the classes that implement several service interfaces
	 * @param[in] Service_ The service implementation class
	 * @param[in] ServiceInterface_ The service interface
	 * @param[in] ... A pointer to an ICreator object for the service implementation
	 * @par Example:
	 * @code
	 * // emma3::VideoOutput implements both IVideoOutput and IAnalogVideoEncoder services
	 * TOOLKIT_DEFINE_SERVICE_CREATOR_EX(emma3::VideoOutput, IAnalogVideoEncoder, new SingleInstanceCreator<IAnalogVideoEncoder, emma3::VideoOutput>());
	 * @endcode
	 */
#define TOOLKIT_DEFINE_SERVICE_CREATOR_EX(Service_, ServiceInterface_, ...) \
		template < > \
		stingray::ServiceProvider<ServiceInterface_>::IServiceCreatorPtr	\
			stingray::ServiceProvider<ServiceInterface_>::s_serviceCreator(new stingray::ServiceCreator<Service_, ServiceInterface_>(stingray::shared_ptr<stingray::ICreator<ServiceInterface_> >(__VA_ARGS__)))


	template < typename ServiceInterface >
	class ServiceProvider;

	struct IServiceInfo
	{
		virtual ~IServiceInfo() { }

		virtual void InitDependencies() const = 0;
		virtual std::string GetServiceTypeName() const = 0;
	};

	template < typename I >
	struct IServiceCreator : public IServiceInfo
	{
		virtual shared_ptr<I> Create() const = 0;
	};

#ifndef DOXYGEN_PREPROCESSOR

	namespace Detail
	{

		class ServiceReferencesInfoException : public std::runtime_error
		{
		private:
			shared_ptr<std::string>		_message;

		public:
			ServiceReferencesInfoException(const std::string& message)
				:	std::runtime_error(message),
					_message(new std::string(message))
			{ }

			virtual ~ServiceReferencesInfoException() throw() { }

			void AppendDependencyName(const std::string& dependencyName)
			{ (*_message) += "\n  referenced from " + dependencyName; }

			virtual const char* what() const throw()
			{ return _message->c_str(); } // TODO: this may cause problems!
		};

		template<typename T>
		struct service_holder_ptr
		{
			TOOLKIT_NONCOPYABLE(service_holder_ptr);

		private:
			volatile bool& _destroyedFlag;
			shared_ptr<T> _ptr;

		public:
			service_holder_ptr(volatile bool& destroyedFlag) : _destroyedFlag(destroyedFlag)
			{ _destroyedFlag =  false; }
			~service_holder_ptr()
			{
				TRACER;
				ServiceProvider<T>::OnServiceDestroyed();
				_destroyedFlag = true;
				_ptr.reset();
			}

			shared_ptr<T>& GetPtr() { return _ptr; }
		};
	}

#endif


	struct NotSupportedServiceException : public Detail::ServiceReferencesInfoException
	{
		NotSupportedServiceException(const std::string& serviceName)
			:	Detail::ServiceReferencesInfoException("The service '" + serviceName + "' is not supported on this platform!")
		{ }
	};

	struct CyclicServiceDependencyException : public Detail::ServiceReferencesInfoException
	{
		CyclicServiceDependencyException(const std::string& dependencyName)
			:	Detail::ServiceReferencesInfoException("Cyclic service dependency: " + dependencyName + " depends on itself!")
		{ }
	};

	struct ServiceCreationFailedException : public Detail::ServiceReferencesInfoException
	{
		ServiceCreationFailedException(const std::string& serviceName)
			:	Detail::ServiceReferencesInfoException("Service '" + serviceName + "' could not be created!")
		{ }
	};

#ifndef DOXYGEN_PREPROCESSOR

	template < typename ServiceInterface >
	struct NotSupportedServiceCreator : public virtual IServiceCreator<ServiceInterface>
	{
		virtual void InitDependencies() const
		{ }

		virtual shared_ptr<ServiceInterface> Create() const
		{ throw NotSupportedServiceException(GetServiceTypeName()); }

		virtual std::string GetServiceTypeName() const
		{ return Demangle(typeid(ServiceInterface).name()); }
	};

	namespace Detail
	{
		template < typename Dependency, bool IsSingleton = IsSingleton<Dependency>::Value >
		struct ServiceDependencyInitializer
		{
			static void Call()
			{ ServiceProvider<Dependency>::Get(); }
		};

		template < typename Dependency >
		struct ServiceDependencyInitializer<Dependency, true>
		{
			static void Call()
			{ Dependency::ConstInstance(); }
		};
	}

	template < typename T, typename ServiceInterface = typename T::ServiceInterface>
	class ServiceCreator : public virtual IServiceCreator<ServiceInterface>
	{
		typedef shared_ptr<ICreator<ServiceInterface> >	ICreatorPtr;

	private:
		ICreatorPtr	_creator;

	public:
		explicit ServiceCreator(const ICreatorPtr& creator)
			: _creator(creator)
		{ }

		virtual void InitDependencies() const
		{ ForEachInTypeList<typename T::Dependencies, InitDependency>::Do(); }

		virtual shared_ptr<ServiceInterface> Create() const { return _creator->Create(); }

		virtual std::string GetServiceTypeName() const
		{ return Demangle(typeid(T).name()); }

	private:
		template < typename Dependency >
		struct InitDependency
		{
			static void Call()
			{ Detail::ServiceDependencyInitializer<Dependency>::Call(); }
		};
	};


	class ServiceProviderBase
	{
	protected:
		typedef shared_ptr<IServiceInfo>		IServiceInfoPtr;

		typedef IServiceInfoPtr (*GetServiceInfoFunc)();
		typedef const Mutex& (*GetMutexFunc)();
		typedef void (*CreateServiceFunc)();
		typedef bool (*IsDestroyedFunc)();

	public:
		static void Get(GetServiceInfoFunc		getServiceInfo,
						GetMutexFunc			getMutex,
						CreateServiceFunc		createService,
						IsDestroyedFunc			isDestroyed,
						const signal<void()>&	onServiceCreated,
						volatile bool&			startedCreating,
						volatile bool&			created,
						volatile bool&			creationFailed)
		{
			if (!created)
			{
				MutexLock l(getMutex());
				if (!created)
				{
					if (creationFailed)
						throw ServiceCreationFailedException(getServiceInfo()->GetServiceTypeName());

					if (startedCreating)
						throw CyclicServiceDependencyException(getServiceInfo()->GetServiceTypeName());

					startedCreating = true;

					try
					{ getServiceInfo()->InitDependencies(); }
					catch (Detail::ServiceReferencesInfoException& ex)
					{
						creationFailed = true;
						startedCreating = false;
						ex.AppendDependencyName(getServiceInfo()->GetServiceTypeName());
						throw;
					}
					catch (...)
					{
						creationFailed = true;
						startedCreating = false;
						throw;
					}

					try
					{ createService(); }
					catch (...)
					{
						creationFailed = true;
						startedCreating = false;
						throw;
					}

					created = true;
					onServiceCreated();
				}
			}
			TOOLKIT_CHECK(!isDestroyed(), "Service " + getServiceInfo()->GetServiceTypeName() + " already destroyed!");
		}
	};


	namespace Detail
	{
		struct ServiceProviderStaticStuff
		{
			Mutex				_mutex;
			volatile bool 		_destroyedFlag;
			signal<void()>		OnServiceCreated;
			signal<void()>		OnServiceDestroyed;

			ServiceProviderStaticStuff(void (*onServiceCreatedPopulator)(const function<void()>&), void (*onServiceDestroyedPopulator)(const function<void()>&))
				: _destroyedFlag(false), OnServiceCreated(onServiceCreatedPopulator), OnServiceDestroyed(onServiceDestroyedPopulator)
			{ }
		};
	}


	template < typename ServiceInterface >
	class ServiceProvider : protected ServiceProviderBase
	{
	public:
		typedef shared_ptr<IServiceCreator<ServiceInterface> >		IServiceCreatorPtr;

	private:
		static IServiceCreatorPtr					s_serviceCreator;
		static Detail::ServiceProviderStaticStuff	s_staticStuff;


		static IServiceInfoPtr DoGetServiceInfo()			{ return s_serviceCreator; }
		static const Mutex& DoGetMutex()					{ return s_staticStuff._mutex; }
		static void DoCreateService()
		{
			shared_ptr<ServiceInterface> inst;
			{
				SystemProfiler sp("[ServiceProvider] Creating " + s_serviceCreator->GetServiceTypeName() + " service", 30, 100);
				inst = s_serviceCreator->Create();
			}
			GetInstancePtr() = inst;
		}

	public:
		static signal<void()>&		OnServiceCreated;
		static signal<void()>&		OnServiceDestroyed;

	public:
		static IServiceCreatorPtr GetCreator()
		{ return s_serviceCreator; }

		static void SetCreator(const IServiceCreatorPtr& serviceCreator)
		{ s_serviceCreator = serviceCreator; }

		static bool IsAlive()
		{ return IsCreated() && !IsDestroyed(); }

		static ServiceInterface& Get()
		{
			static volatile bool started_creating = false;
			volatile bool& created = GetCreatedFlag();
			static volatile bool creation_failed = false;

			ServiceProviderBase::Get(&ServiceProvider::DoGetServiceInfo, &ServiceProvider::DoGetMutex, &ServiceProvider::DoCreateService, &ServiceProvider::IsDestroyed,
				OnServiceCreated, started_creating, created, creation_failed);

			return *GetInstancePtr();
		}

	private:
		static void OnServiceCreatedPopulator(const function<void()>& slot)
		{ if (IsCreated()) slot(); }

		static void OnServiceDestroyedPopulator(const function<void()>& slot)
		{ if (IsDestroyed()) slot(); }

		static volatile bool& GetCreatedFlag()	{ static volatile bool flag = false; return flag; }
		static bool IsCreated()					{ return GetCreatedFlag(); }
		static bool IsDestroyed()				{ return s_staticStuff._destroyedFlag; } // if service wasn't is created, returns undefined value


		static shared_ptr<ServiceInterface>& GetInstancePtr()
		{
			static Detail::service_holder_ptr<ServiceInterface> inst(s_staticStuff._destroyedFlag);
			return inst.GetPtr();
		}
	};

	template < typename T > Detail::ServiceProviderStaticStuff ServiceProvider<T>::s_staticStuff(&ServiceProvider<T>::OnServiceCreatedPopulator, &ServiceProvider<T>::OnServiceDestroyedPopulator);
	template < typename T > signal<void()>& ServiceProvider<T>::OnServiceCreated(ServiceProvider<T>::s_staticStuff.OnServiceCreated);
	template < typename T > signal<void()>& ServiceProvider<T>::OnServiceDestroyed(ServiceProvider<T>::s_staticStuff.OnServiceDestroyed);

#else

	/**
	 * @brief A class that holds a static instance of a service
	 * @tparam ServiceInterface The service interface
	 * @par Example:
	 * @code
	 * // This line will initialize the dependencies and the service itself when invoked the first time, and then return the instance of IHDMI
	 * ServiceProvider<IHDMI>::Get().EnableHdcpAuthentication();
	 * // Usually, we have typedefs for service providers, so the same code would look like this:
	 * HDMIProvider::Get().EnableHdcpAuthentication();
	 * @endcode
	 */
	template < typename ServiceInterface >
	class ServiceProvider
	{
	public:
		typedef shared_ptr<IServiceCreator<ServiceInterface> >		IServiceCreatorPtr;

		/**
		 * @brief This signal is invoked when the service instance is created. You should use it when you need a 'weak' dependency.
		 */
		static signal<void()>&		OnServiceCreated;

		/**
		 * @brief This signal is invoked when the service instance is destroyed. You should use it when you need a 'weak' dependency.
		 */
		static signal<void()>&		OnServiceDestroyed;

		/**
		 * @brief Gets a pointer to a service implementation creator in runtime. You should use TOOLKIT_GET_SERVICE_CREATOR macro.
		 * @returns A shared_ptr to the ICreator object for the service implementation
		 */
		static IServiceCreatorPtr GetCreator();

		/**
		 * @brief Sets a pointer to a service implementation creator in runtime. You should use TOOLKIT_SET_SERVICE_CREATOR macro.
		 * @param[in] serviceCreator A pointer to an IServiceCreator object for the service implementation
		 */
		static void SetCreator(const IServiceCreatorPtr& serviceCreator);

		/**
		 * @returns true if a service is already created and not yet destroyed, false otherwise
		 */
		static bool IsAlive();

		/**
		 * @brief This method initializes dependencies and the service implementation, and return a reference to the service. If either a service or its dependencies cannnot be created, throws an exception.
		 * @returns A reference to a service instance
		 */
		static ServiceInterface& Get();
	};

#endif

	/** @} */

}


#endif
