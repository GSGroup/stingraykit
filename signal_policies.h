#ifndef STINGRAY_TOOLKIT_SIGNAL_POLICIES_H
#define STINGRAY_TOOLKIT_SIGNAL_POLICIES_H


#include <stingray/threads/Thread.h>
#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/function.h>
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
					void Lock() { }
					void Unlock() { }
				};

				typedef GenericMutexLock<DummyMutex>	LockType;
				DummyMutex GetSync() const { return DummyMutex(); }
			};
		}


		namespace exception_handling
		{
			typedef function<void(const std::exception&)>	ExceptionHandlerFunc;

			inline void DefaultSignalExceptionHandler(const std::exception &ex)
			{ Logger::Error() << "Uncaught exception in signal handler: " << diagnostic_information(ex); }


			struct Configurable
			{
				Configurable() : _exceptionHandler(&DefaultSignalExceptionHandler) {}
				Configurable(const ExceptionHandlerFunc &ehf) : _exceptionHandler(ehf) {}
				const ExceptionHandlerFunc& GetExceptionHandler() const { return _exceptionHandler; }

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
