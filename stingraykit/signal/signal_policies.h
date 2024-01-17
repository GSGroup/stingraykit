#ifndef STINGRAYKIT_SIGNAL_SIGNAL_POLICIES_H
#define STINGRAYKIT_SIGNAL_SIGNAL_POLICIES_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/Thread.h>

namespace stingray
{

	struct ConnectionPolicy
	{
		STINGRAYKIT_ENUM_VALUES(SyncOnly, AsyncOnly, Any);
		STINGRAYKIT_DECLARE_ENUM_CLASS(ConnectionPolicy);
	};


	namespace signal_policies
	{

		namespace creation
		{

			struct Default
			{
				template < typename ImplType >
				static self_count_ptr<ImplType> CtorCreate()
				{ return make_self_count_ptr<ImplType>(); }

				template < typename ImplType >
				static void LazyCreate(self_count_ptr<ImplType>& impl)
				{ }
			};

			struct Lazy
			{
				template < typename ImplType >
				static self_count_ptr<ImplType> CtorCreate()
				{ return null; }

				template < typename ImplType >
				static void LazyCreate(self_count_ptr<ImplType>& impl)
				{ if (!impl) impl = make_self_count_ptr<ImplType>(); }
			};

		}


		namespace threading
		{

			struct DummyMutex
			{
				void Lock() const { }
				void Unlock() const { }
			};
			typedef GenericMutexLock<DummyMutex>	DummyLock;

			struct Multithreaded
			{
				static const bool IsThreadsafe = true;

			private:
				Mutex	_mutex;

			public:
				const Mutex& GetSync() const { return _mutex; }
			};

			struct Threadless
			{
				static const bool IsThreadsafe = false;

				DummyMutex GetSync() const { return DummyMutex(); }
			};

			struct ExternalMutexPointer
			{
				static const bool IsThreadsafe = true;

			private:
				shared_ptr<const Mutex>	_mutex;

			public:
				explicit ExternalMutexPointer(const shared_ptr<const Mutex>& mutex) : _mutex(mutex) { }

				const Mutex& GetSync() const { return *_mutex; }
			};

		}


		namespace exception_handling
		{

			typedef function<void (const std::exception&)>	ExceptionHandlerFunc;

			void DefaultSignalExceptionHandler(const std::exception& ex);

			class Configurable
			{
			private:
				ExceptionHandlerFunc	_exceptionHandler;

			public:
				Configurable() : _exceptionHandler(&DefaultSignalExceptionHandler) { }
				explicit Configurable(const ExceptionHandlerFunc& ehf) : _exceptionHandler(ehf) { }

				const ExceptionHandlerFunc& GetExceptionHandler() const { return _exceptionHandler; }
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
				void SendCurrentStateImpl(const function<Signature_>& slot) const { }
			};

			class Configurable
			{
			private:
				optional<function_storage>		_sendCurrentState;

			public:
				Configurable() { }

				template < typename Signature_ >
				explicit Configurable(const function<void(const function<Signature_>&)>& func) : _sendCurrentState(function_storage(func)) { }

				template < typename Signature_ >
				void SendCurrentStateImpl(const function<Signature_>& slot) const
				{
					if (_sendCurrentState)
						_sendCurrentState->ToFunction<void(const function<Signature_>&)>()(slot);
				}
			};

		}


		namespace connection_policy_control
		{

			struct Null
			{
				ConnectionPolicy DoGetConnectionPolicy() const { return ConnectionPolicy::Any; }
			};

			struct Checked
			{
			private:
				ConnectionPolicy	_val;

			public:
				explicit Checked(ConnectionPolicy val = ConnectionPolicy::Any) : _val(val) { }

				ConnectionPolicy DoGetConnectionPolicy() const { return _val; }
			};

		};

	}
}

#endif
