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


#define TOOLKIT_SERVICE_NOT_SUPPORTED(ServiceInterface_) \
		template < > \
		stingray::ServiceProvider<ServiceInterface_>::IServiceCreatorPtr	\
			stingray::ServiceProvider<ServiceInterface_>::s_serviceCreator(new stingray::NotSupportedServiceCreator<ServiceInterface_>)

#define TOOLKIT_DEFINE_SERVICE_CREATOR(Service_, ...) \
		namespace { stingray::CompileTimeAssert<stingray::GetTypeListLength<Service_::ServiceInterfaces>::Value == 1> PLEASE_USE_DEFINE_SERVICE_CREATOR_EX_IF_YOU_IMPLEMENT_MORE_THAN_ONE_INTERFACE; } \
		template < > \
		stingray::ServiceProvider<Service_::ServiceInterfaces::ValueT>::IServiceCreatorPtr	\
			stingray::ServiceProvider<Service_::ServiceInterfaces::ValueT>::s_serviceCreator(new stingray::ServiceCreator<Service_, Service_::ServiceInterfaces::ValueT>(stingray::shared_ptr<stingray::ICreator<Service_::ServiceInterfaces::ValueT> >(__VA_ARGS__)))

#define TOOLKIT_GET_SERVICE_CREATOR(ServiceInterface_) \
		stingray::ServiceProvider<ServiceInterface_>::GetCreator()

#define TOOLKIT_SET_SERVICE_CREATOR(Service_, ...) \
		stingray::ServiceProvider<Service_::ServiceInterfaces::ValueT>::SetCreator(stingray::make_shared<stingray::ServiceCreator<Service_, Service_::ServiceInterfaces::ValueT> >(stingray::shared_ptr<stingray::ICreator<Service_::ServiceInterfaces::ValueT> >(__VA_ARGS__)))

#define TOOLKIT_DEFINE_SERVICE_CREATOR_EX(Service_, ServiceInterface_, ...) \
		template < > \
		stingray::ServiceProvider<ServiceInterface_>::IServiceCreatorPtr	\
			stingray::ServiceProvider<ServiceInterface_>::s_serviceCreator(new stingray::ServiceCreator<Service_, ServiceInterface_>(stingray::shared_ptr<stingray::ICreator<ServiceInterface_> >(__VA_ARGS__)))

namespace stingray
{

	/*! \cond GS_INTERNAL */

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

	/*! \endcond */

}


#endif
