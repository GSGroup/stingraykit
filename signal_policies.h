#ifndef STINGRAY_TOOLKIT_SIGNAL_POLICIES_H
#define STINGRAY_TOOLKIT_SIGNAL_POLICIES_H


#include <stingray/threads/Thread.h>
#include <stingray/toolkit/function/function.h>
#include <stingray/toolkit/optional.h>


namespace stingray
{

	struct ConnectionPolicy
	{
		TOOLKIT_ENUM_VALUES(SyncOnly, AsyncOnly, Any);
		TOOLKIT_DECLARE_ENUM_CLASS(ConnectionPolicy);
	};


	namespace signal_policies
	{

		namespace creation
		{
			struct Default
			{
				template <typename ImplType>
				static self_count_ptr<ImplType> CtorCreate()
				{ return self_count_ptr<ImplType>(new ImplType()); }

				template <typename ImplType>
				static void LazyCreate(self_count_ptr<ImplType>& impl)
				{ }
			};

			struct Lazy
			{
				template <typename ImplType>
				static self_count_ptr<ImplType> CtorCreate()
				{ return null; }

				template <typename ImplType>
				static void LazyCreate(self_count_ptr<ImplType>& impl)
				{ if (!impl) impl.reset(new ImplType()); }
			};
		}


		namespace threading
		{
			struct Multithreaded
			{
				typedef MutexLock		LockType;
				const Mutex& GetSync() const { return _mutex; }

			private:
				Mutex	_mutex;
			};

			struct Threadless
			{
				struct DummyMutex
				{
					void Lock() const { }
					void Unlock() const { }
				};

				typedef GenericMutexLock<DummyMutex>	LockType;
				DummyMutex GetSync() const { return DummyMutex(); }
			};

			struct ExternalMutex
			{
				typedef MutexLock		LockType;

			private:
				const Mutex&	_mutex;

			public:
				ExternalMutex(const Mutex& mutex) : _mutex(mutex)
				{ }

				const Mutex& GetSync() const { return _mutex; }
			};

			struct ExternalMutexPointer
			{
				typedef MutexLock		LockType;

			private:
				shared_ptr<Mutex>	_mutex;

			public:
				ExternalMutexPointer(const shared_ptr<Mutex>& mutex) : _mutex(mutex)
				{ }

				const Mutex& GetSync() const { return *_mutex; }
			};
		}


		namespace exception_handling
		{
			typedef function<void(const std::exception&)>	ExceptionHandlerFunc;

			void DefaultSignalExceptionHandler(const std::exception &ex);

			struct Configurable
			{
				Configurable() : _exceptionHandler(&DefaultSignalExceptionHandler)		{}
				Configurable(const ExceptionHandlerFunc &ehf) : _exceptionHandler(ehf)	{}
				const ExceptionHandlerFunc& GetExceptionHandler() const					{ return _exceptionHandler; }

			private:
				ExceptionHandlerFunc							_exceptionHandler;
			};

			struct Null
			{
				ExceptionHandlerFunc GetExceptionHandler() const { return &DefaultSignalExceptionHandler; }
			};
		}


		namespace populators
		{
			struct Null
			{
				template < typename Signature_ >
				void SendCurrentState(const function<Signature_>& slot) const { }
			};

			struct Configurable
			{
				Configurable() { }

				template < typename Signature_ >
				Configurable(const function<void(const function<Signature_>&)>& func) : _sendCurrentState(function_storage(func)) { }

				template < typename Signature_ >
				void SendCurrentState(const function<Signature_>& slot) const
				{
					if (_sendCurrentState)
						_sendCurrentState->ToFunction<void(const function<Signature_>&)>()(slot);
				}

			private:
				optional<function_storage>		_sendCurrentState;
			};
		}


		namespace connection_policy_control
		{
			struct Null
			{
				ConnectionPolicy GetConnectionPolicy() const { return ConnectionPolicy::Any; }
			};

			struct Checked
			{
				Checked(ConnectionPolicy val = ConnectionPolicy::Any) : _val(val) { }

				ConnectionPolicy GetConnectionPolicy() const { return _val; }

			private:
				ConnectionPolicy	_val;
			};
		};

	}
}

#endif
