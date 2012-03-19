#ifndef __GS_DVRLIB_TOOLKIT_SERVICEPROVIDER_H__
#define __GS_DVRLIB_TOOLKIT_SERVICEPROVIDER_H__

#include <list>
#include <typeinfo>

#include <dvrlib/toolkit/exception.h>
#include <dvrlib/toolkit/ICreator.h>
#include <dvrlib/toolkit/ServiceTraits.h>
#include <dvrlib/toolkit/signals.h>
#include <dvrlib/toolkit/MetaProgramming.h>
#include <dvrlib/threads/Thread.h>


#define TOOLKIT_SERVICE_NOT_SUPPORTED(ServiceInterface_) \
		template < > \
		ServiceProvider<ServiceInterface_>::IServiceCreatorPtr	\
			ServiceProvider<ServiceInterface_>::s_serviceCreator(new dvrlib::NotSupportedServiceCreator<ServiceInterface_>)

#define TOOLKIT_DEFINE_SERVICE_CREATOR(Service_, ...) \
		namespace { CompileTimeAssert<GetTypeListLength<Service_::ServiceInterfaces>::Value == 1> PLEASE_USE_DEFINE_SERVICE_CREATOR_EX_IF_YOU_IMPLEMENT_MORE_THAN_ONE_INTERFACE; } \
		template < > \
		ServiceProvider<Service_::ServiceInterfaces::ValueT>::IServiceCreatorPtr	\
			ServiceProvider<Service_::ServiceInterfaces::ValueT>::s_serviceCreator(new ServiceCreator<Service_, Service_::ServiceInterfaces::ValueT>(shared_ptr<ICreator<Service_::ServiceInterfaces::ValueT> >(__VA_ARGS__)))

#define TOOLKIT_SET_SERVICE_CREATOR(Service_, ...) \
		ServiceProvider<Service_::ServiceInterfaces::ValueT>::SetCreator(make_shared<ServiceCreator<Service_, Service_::ServiceInterfaces::ValueT> >(shared_ptr<ICreator<Service_::ServiceInterfaces::ValueT> >(__VA_ARGS__)))

#define TOOLKIT_DEFINE_SERVICE_CREATOR_EX(Service_, ServiceInterface_, ...) \
		template < > \
		ServiceProvider<ServiceInterface_>::IServiceCreatorPtr	\
			ServiceProvider<ServiceInterface_>::s_serviceCreator(new ServiceCreator<Service_, ServiceInterface_>(dvrlib::shared_ptr<ICreator<ServiceInterface_> >(__VA_ARGS__)))

namespace dvrlib
{

	/*! \cond GS_INTERNAL */

	template < typename ServiceInterface >
	class ServiceProvider;

	template < typename I >
	struct IServiceCreator
	{
		virtual ~IServiceCreator() { }

		virtual void InitDependencies() const = 0;
		virtual shared_ptr<I> Create() const = 0;
		virtual std::string GetServiceTypeName() const = 0;
	};


	namespace Detail
	{

		class ServiceReferencesInfoException : public std::runtime_error
		{
		private:
			bool						_firstService;
			shared_ptr<std::string>		_message;

		public:
			ServiceReferencesInfoException(const std::string& message, const std::string& dependencyName)
				:	std::runtime_error(message),
					_firstService(true),
					_message(new std::string(message))
			{ AppendDependencyName(dependencyName); }

			virtual ~ServiceReferencesInfoException() throw() { }

			void AppendDependencyName(const std::string& dependencyName)
			{
				(*_message) += std::string(_firstService ? "" : " referenced from") + "\n" + dependencyName;

				if (_firstService)
					_firstService = false;
			}

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
			service_holder_ptr(volatile bool& destroyedFlag) : _destroyedFlag(destroyedFlag) { _destroyedFlag =  false; }
			~service_holder_ptr() { ServiceProvider<T>::OnServiceDestroyed(); _destroyedFlag = true; }
			shared_ptr<T>& GetPtr() { return _ptr; }
		};
	}


	struct NotSupportedServiceException : public Detail::ServiceReferencesInfoException
	{
		NotSupportedServiceException(const std::string& serviceName)
			:	Detail::ServiceReferencesInfoException("The service '" + serviceName + "'is not supported on this platform!", serviceName)
		{ }
	};

	struct CyclicServiceDependencyException : public Detail::ServiceReferencesInfoException
	{
		CyclicServiceDependencyException(const std::string& dependencyName)
			:	Detail::ServiceReferencesInfoException("Cyclic service dependency!", dependencyName)
		{ }
	};


	template < typename ServiceInterface >
	struct NotSupportedServiceCreator : public virtual IServiceCreator<ServiceInterface>
	{
		virtual void InitDependencies() const
		{ }

		virtual shared_ptr<ServiceInterface> Create() const
		{ TOOLKIT_THROW(NotSupportedServiceException(GetServiceTypeName())); }

		virtual std::string GetServiceTypeName() const
		{ return typeid(ServiceInterface).name(); }
	};


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
		{ return typeid(T).name(); }

	private:
		template < typename Dependency >
		struct InitDependency
		{
			static void Call()
			{ ServiceProvider<Dependency>::Get(); }
		};
	};


	template < typename ServiceInterface >
	class ServiceProvider
	{
		typedef shared_ptr<IServiceCreator<ServiceInterface> >		IServiceCreatorPtr;

	private:
		static IServiceCreatorPtr	s_serviceCreator;
		static Mutex				s_mutex;

	public:
		static signal<void()>		OnServiceCreated;
		static signal<void()>		OnServiceDestroyed;

	public:
		static void SetCreator(const IServiceCreatorPtr& serviceCreator)
		{ s_serviceCreator = serviceCreator; }

		static bool IsAlive()
		{ return IsCreated() && !IsDestroyed(); }

		static ServiceInterface& Get()
		{
			static volatile bool started_creating = false;
			volatile bool& created = GetCreatedFlag();

			if (!created)
			{
				MutexLock l(s_mutex);
				if (!created)
				{
					if (started_creating)
						TOOLKIT_THROW(CyclicServiceDependencyException(s_serviceCreator->GetServiceTypeName()));

					started_creating = true;

					try
					{ s_serviceCreator->InitDependencies(); }
					catch (Detail::ServiceReferencesInfoException& ex)
					{
						started_creating = false;
						ex.AppendDependencyName(s_serviceCreator->GetServiceTypeName());
						throw;
					}
					catch (...)
					{
						started_creating = false;
						throw;
					}

					try
					{
						shared_ptr<ServiceInterface> inst = s_serviceCreator->Create();
						GetInstancePtr() = inst;
					}
					catch (...)
					{
						started_creating = false;
						throw;
					}

					created = true;
					OnServiceCreated();
				}
			}

			return *GetInstancePtr();
		}

	private:
		static void OnServiceCreatedPopulator(const function<void()>& slot)
		{ if (IsCreated()) slot(); }

		static void OnServiceDestroyedPopulator(const function<void()>& slot)
		{ if (IsDestroyed()) slot(); }

		static volatile bool& GetCreatedFlag()
		{
			static volatile bool flag = false;
			return flag;
		};
		static bool IsCreated()
		{ return GetCreatedFlag(); }
		static bool IsDestroyed() // if service wasn't is created, returns undefined value
		{ return s_destroyedFlag; }


		static volatile bool s_destroyedFlag;

		static shared_ptr<ServiceInterface>& GetInstancePtr()
		{
			static Detail::service_holder_ptr<ServiceInterface> inst(s_destroyedFlag);
			return inst.GetPtr();
		}
	};

	template < typename T > signal<void()> ServiceProvider<T>::OnServiceCreated(&ServiceProvider<T>::OnServiceCreatedPopulator);
	template < typename T > signal<void()> ServiceProvider<T>::OnServiceDestroyed(&ServiceProvider<T>::OnServiceDestroyedPopulator);

	template < typename ServiceInterface >
	Mutex ServiceProvider<ServiceInterface>::s_mutex;
	template < typename ServiceInterface >
	volatile bool ServiceProvider<ServiceInterface>::s_destroyedFlag;



	/*! \endcond */

}


#endif
