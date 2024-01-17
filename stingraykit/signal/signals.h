#ifndef STINGRAYKIT_SIGNAL_SIGNALS_H
#define STINGRAYKIT_SIGNAL_SIGNALS_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/IntrusiveList.h>
#include <stingraykit/signal/signal_connector.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

#ifndef DOXYGEN_PREPROCESSOR

	namespace Detail
	{

		template < typename Signature, typename ExceptionHandlerFunc >
		class ExceptionHandlerWrapper;

		template < typename... Ts, typename ExceptionHandlerFunc >
		class ExceptionHandlerWrapper<void (Ts...), ExceptionHandlerFunc> : public function_info<void (Ts...)>
		{
		public:
			typedef function_info<void (Ts...)>		BaseType;

			typedef typename BaseType::ParamTypes	ParamTypes;
			typedef typename BaseType::Signature	Signature;

			typedef function<Signature>				FuncType;

		private:
			FuncType				_func;
			ExceptionHandlerFunc	_exFunc;

		public:
			ExceptionHandlerWrapper(const FuncType& func, const ExceptionHandlerFunc& exFunc) : _func(func), _exFunc(exFunc)
			{ }

			void operator () (Ts... args) const
			{
				try
				{ _func(args...); }
				catch (std::exception& ex)
				{ _exFunc(ex); }
			}
		};


#define WRAP_EXCEPTION_HANDLING(exceptionHandler, ...) \
		try \
		{ __VA_ARGS__; } \
		catch (const std::exception& ex) \
		{ exceptionHandler(ex); }


		template < bool IsThreadsafe >
		class SignalImplBase : public ISignalConnector
		{
			typedef self_count_ptr<SignalImplBase>												ImplPtr;

		protected:
			class CancellableStorage : public IntrusiveListNode<CancellableStorage>
			{
			private:
				function_storage		_functionStorage;
				FutureExecutionTester	_tester;

			public:
				CancellableStorage(const function_storage& func, const FutureExecutionTester& tester)
					: _functionStorage(func), _tester(tester)
				{ }

				template < typename Signature_, typename... Ts >
				void Invoke(Ts... args) const
				{
					LocalExecutionGuard guard(_tester);
					if (guard)
						_functionStorage.ToFunction<Signature_>()(args...);
				}
			};

			class ThreadlessStorage : public IntrusiveListNode<ThreadlessStorage>
			{
			private:
				function_storage		_functionStorage;

			public:
				ThreadlessStorage(const function_storage& func, const FutureExecutionTester& tester)
					: _functionStorage(func)
				{ STINGRAYKIT_CHECK(tester.IsDummy(), "ThreadlessStorage can't be used with real tokens!"); }

				template < typename Signature_, typename... Ts >
				void Invoke(Ts... args) const
				{ _functionStorage.ToFunction<Signature_>()(args...); }
			};

			typedef typename If<IsThreadsafe, CancellableStorage, ThreadlessStorage>::ValueT	FuncStorageType;
			typedef IntrusiveList<FuncStorageType>												Handlers;
			typedef inplace_vector<FuncStorageType, 16>											LocalHandlersCopy;

			typedef signal_policies::threading::DummyMutex										DummyMutex;
			typedef signal_policies::threading::DummyLock										DummyLock;
			typedef typename If<IsThreadsafe, const Mutex&, DummyMutex>::ValueT					MutexRefType;
			typedef typename If<IsThreadsafe, MutexLock, DummyLock>::ValueT						LockType;

		private:
			class Connection : public IToken
			{
			private:
				ImplPtr				_impl;
				FuncStorageType		_handler;
				TaskLifeToken		_token;

			public:
				Connection(const ImplPtr& impl, const function_storage& func, const FutureExecutionTester& invokeTester, const TaskLifeToken& connectionToken)
					: _impl(impl), _handler(func, invokeTester), _token(connectionToken)
				{ _impl->AddHandler(_handler); }

				virtual ~Connection()
				{
					_impl->RemoveHandler(_handler);
					_token.Release();
				}
			};

		protected:
			Handlers	_handlers;

		public:
			virtual TaskLifeToken CreateSyncToken() const	{ return IsThreadsafe ? TaskLifeToken() : TaskLifeToken::CreateDummyTaskToken(); }
			virtual TaskLifeToken CreateAsyncToken() const	{ return TaskLifeToken(); }

			virtual Token Connect(const function_storage& func, const FutureExecutionTester& invokeTester, const TaskLifeToken& connectionToken, bool sendCurrentState)
			{
				LockType l(DoGetSync());
				if (sendCurrentState)
					DoSendCurrentState(func);

				return MakeToken<Connection>(ImplPtr(self_count_ptr_from_this(), static_cast_tag()), func, invokeTester, connectionToken);
			}

			virtual void SendCurrentState(const function_storage& slot) const
			{
				LockType l(DoGetSync());
				DoSendCurrentState(slot);
			}

		private:
			void AddHandler(FuncStorageType& handler)
			{
				// mutex is locked in Connect
				_handlers.push_back(handler);
			}

			void RemoveHandler(FuncStorageType& handler)
			{
				LockType l(DoGetSync());
				_handlers.erase(handler);
			}

		protected:
			void CopyHandlersToLocal(LocalHandlersCopy& localCopy) const
			{ std::copy(_handlers.begin(), _handlers.end(), std::back_inserter(localCopy)); }

		protected:
			virtual MutexRefType DoGetSync() const = 0;
			virtual void DoSendCurrentState(const function_storage& slot) const = 0;
		};


		template < typename Signature_, typename ThreadingPolicy_, typename ExceptionPolicy_, typename PopulatorsPolicy_, typename ConnectionPolicyControl_ >
		class SignalImpl;

		template < typename... Ts, typename ThreadingPolicy_, typename ExceptionPolicy_, typename PopulatorsPolicy_, typename ConnectionPolicyControl_ >
		class SignalImpl<void (Ts...), ThreadingPolicy_, ExceptionPolicy_, PopulatorsPolicy_, ConnectionPolicyControl_>
			:	public ThreadingPolicy_, public ExceptionPolicy_, public PopulatorsPolicy_, public ConnectionPolicyControl_, public SignalImplBase<ThreadingPolicy_::IsThreadsafe>
		{
			typedef SignalImplBase<ThreadingPolicy_::IsThreadsafe>	base;

			typedef void Signature(Ts...);

			typedef function<void(const std::exception&)>			ExceptionHandlerFunc;
			typedef function<void(const function<Signature>&)>		PopulatorFunc;

		public:
			SignalImpl() { }

			explicit SignalImpl(ConnectionPolicy connectionPolicy)
				: ConnectionPolicyControl_(connectionPolicy)
			{ }

			SignalImpl(NullPtrType, const ExceptionHandlerFunc& exceptionHandler)
				: ExceptionPolicy_(exceptionHandler)
			{ }

			SignalImpl(NullPtrType, const ExceptionHandlerFunc& exceptionHandler, ConnectionPolicy connectionPolicy)
				: ExceptionPolicy_(exceptionHandler), ConnectionPolicyControl_(connectionPolicy)
			{ }

			SignalImpl(const PopulatorFunc& sendCurrentState)
				: PopulatorsPolicy_(sendCurrentState)
			{ }

			SignalImpl(const PopulatorFunc& sendCurrentState, ConnectionPolicy connectionPolicy)
				: PopulatorsPolicy_(sendCurrentState), ConnectionPolicyControl_(connectionPolicy)
			{ }

			SignalImpl(const PopulatorFunc& sendCurrentState, const ExceptionHandlerFunc& exceptionHandler)
				: ExceptionPolicy_(exceptionHandler), PopulatorsPolicy_(sendCurrentState)
			{ }

			SignalImpl(const PopulatorFunc& sendCurrentState, const ExceptionHandlerFunc& exceptionHandler, ConnectionPolicy connectionPolicy)
				: ExceptionPolicy_(exceptionHandler), PopulatorsPolicy_(sendCurrentState), ConnectionPolicyControl_(connectionPolicy)
			{ }

			SignalImpl(const ThreadingPolicy_& threadingPolicy)
				: ThreadingPolicy_(threadingPolicy)
			{ }

			SignalImpl(const ThreadingPolicy_& threadingPolicy, ConnectionPolicy connectionPolicy)
				: ThreadingPolicy_(threadingPolicy), ConnectionPolicyControl_(connectionPolicy)
			{ }

			SignalImpl(const ThreadingPolicy_& threadingPolicy, const PopulatorFunc& sendCurrentState)
				: ThreadingPolicy_(threadingPolicy), PopulatorsPolicy_(sendCurrentState)
			{ }

			SignalImpl(const ThreadingPolicy_& threadingPolicy, const PopulatorFunc& sendCurrentState, ConnectionPolicy connectionPolicy)
				: ThreadingPolicy_(threadingPolicy), PopulatorsPolicy_(sendCurrentState), ConnectionPolicyControl_(connectionPolicy)
			{ }

			void InvokeAll(Ts... args) const
			{
				typename base::LocalHandlersCopy localCopy;
				{
					typename base::LockType l(this->GetSync());
					this->CopyHandlersToLocal(localCopy);
				}

				for (typename base::LocalHandlersCopy::const_iterator it = localCopy.begin(); it != localCopy.end(); ++it)
					WRAP_EXCEPTION_HANDLING(this->GetExceptionHandler(), it->template Invoke<Signature, Ts...>(args...));
			}

			virtual ConnectionPolicy GetConnectionPolicy() const { return this->DoGetConnectionPolicy(); }

		private:
			virtual typename base::MutexRefType DoGetSync() const
			{ return this->GetSync(); }

			virtual void DoSendCurrentState(const function_storage& slot) const
			{
				const ExceptionHandlerWrapper<Signature, ExceptionHandlerFunc> wrappedSlot(slot.ToFunction<Signature>(), this->GetExceptionHandler());
				WRAP_EXCEPTION_HANDLING(this->GetExceptionHandler(), this->template SendCurrentStateImpl<Signature>(wrappedSlot));
			}
		};

	}


	class signal_locker
	{
		STINGRAYKIT_NONCOPYABLE(signal_locker);

	private:
		MutexLock	_mutexLock;

	public:
		template < typename SignalType >
		signal_locker(const SignalType& theSignal)
			: _mutexLock(theSignal._impl->GetSync())
		{ }
	};


	template < typename Signature_,
		typename ThreadingPolicy_ = signal_policies::threading::Multithreaded,
		typename ExceptionPolicy_ = signal_policies::exception_handling::Configurable,
		typename PopulatorsPolicy_ = signal_policies::populators::Configurable,
		typename ConnectionPolicyControl_ = signal_policies::connection_policy_control::Checked,
		typename CreationPolicy_ = signal_policies::creation::Default>
	class signal;

	template < typename... Ts, typename ThreadingPolicy_, typename ExceptionPolicy_, typename PopulatorsPolicy_, typename ConnectionPolicyControl_, typename CreationPolicy_ >
	class signal<void (Ts...), ThreadingPolicy_, ExceptionPolicy_, PopulatorsPolicy_, ConnectionPolicyControl_, CreationPolicy_>
	{
		STINGRAYKIT_NONCOPYABLE(signal);

		friend class signal_locker;

	public:
		typedef void Signature(Ts...);

		typedef void											RetType;
		typedef typename function_info<Signature>::ParamTypes	ParamTypes;

	private:
		typedef Detail::SignalImpl<Signature, ThreadingPolicy_, ExceptionPolicy_, PopulatorsPolicy_, ConnectionPolicyControl_>	Impl;
		typedef self_count_ptr<Impl>																							ImplPtr;

		typedef function<void (const std::exception&)>			ExceptionHandlerFunc;
		typedef function<void (const function<Signature>&)>		PopulatorFunc;

		typedef ThreadingPolicy_								ThreadingPolicy;

	public:
		class Invoker : public function_info<Signature>
		{
		private:
			ImplPtr		_impl;
		public:
			Invoker(const ImplPtr& impl) : _impl(impl)
			{ }

			void operator () (Ts... args) const
			{ _impl->InvokeAll(args...); }
		};

	private:
		mutable ImplPtr		_impl;

	public:
		signal() : _impl(CreationPolicy_::template CtorCreate<Impl>()) { }

		signal(NullPtrType, NullPtrType, ConnectionPolicy connectionPolicy) : _impl(make_self_count_ptr<Impl>(connectionPolicy)) { }
		signal(NullPtrType, const ExceptionHandlerFunc& exceptionHandler) : _impl(make_self_count_ptr<Impl>(null, exceptionHandler)) { }
		signal(NullPtrType, const ExceptionHandlerFunc& exceptionHandler, ConnectionPolicy connectionPolicy) : _impl(make_self_count_ptr<Impl>(null, exceptionHandler, connectionPolicy)) { }

		signal(const PopulatorFunc& sendCurrentState) : _impl(make_self_count_ptr<Impl>(sendCurrentState)) { }
		signal(const PopulatorFunc& sendCurrentState, NullPtrType, ConnectionPolicy connectionPolicy) : _impl(make_self_count_ptr<Impl>(sendCurrentState, connectionPolicy)) { }
		signal(const PopulatorFunc& sendCurrentState, const ExceptionHandlerFunc& exceptionHandler) : _impl(make_self_count_ptr<Impl>(sendCurrentState, exceptionHandler)) { }
		signal(const PopulatorFunc& sendCurrentState, const ExceptionHandlerFunc& exceptionHandler, ConnectionPolicy connectionPolicy) : _impl(make_self_count_ptr<Impl>(sendCurrentState, exceptionHandler, connectionPolicy)) { }

		signal(const ThreadingPolicy& threadingPolicy) : _impl(make_self_count_ptr<Impl>(threadingPolicy)) { }
		signal(const ThreadingPolicy& threadingPolicy, NullPtrType, ConnectionPolicy connectionPolicy) : _impl(make_self_count_ptr<Impl>(threadingPolicy, connectionPolicy)) { }
		signal(const ThreadingPolicy& threadingPolicy, const PopulatorFunc& sendCurrentState) : _impl(make_self_count_ptr<Impl>(threadingPolicy, sendCurrentState)) { }
		signal(const ThreadingPolicy& threadingPolicy, const PopulatorFunc& sendCurrentState, ConnectionPolicy connectionPolicy) : _impl(make_self_count_ptr<Impl>(threadingPolicy, sendCurrentState, connectionPolicy)) { }

		void SendCurrentState(const function<Signature>& slot) const
		{
			if (_impl)
				_impl->SendCurrentState(function_storage(slot));
		}

		Token connect(const function<Signature>& slot, bool sendCurrentState = true) const
		{
			CreationPolicy_::template LazyCreate(_impl);

			STINGRAYKIT_CHECK(_impl->GetConnectionPolicy() == ConnectionPolicy::Any || _impl->GetConnectionPolicy() == ConnectionPolicy::SyncOnly, "sync-connect to async-only signal");

			const TaskLifeToken token(_impl->CreateSyncToken());
			return _impl->Connect(function_storage(slot), token.GetExecutionTester(), token, sendCurrentState);
		}

		Token connect(const ITaskExecutorPtr& worker, const function<Signature>& slot, bool sendCurrentState = true) const
		{
			CreationPolicy_::template LazyCreate(_impl);

			STINGRAYKIT_CHECK(_impl->GetConnectionPolicy() == ConnectionPolicy::Any || _impl->GetConnectionPolicy() == ConnectionPolicy::AsyncOnly, "async-connect to sync-only signal");

			const TaskLifeToken token(_impl->CreateAsyncToken());
			return _impl->Connect(function_storage(function<Signature>(MakeAsyncFunction(worker, slot, token.GetExecutionTester()))), null, token, sendCurrentState);
		}

		signal_connector<Signature> connector() const
		{
			CreationPolicy_::template LazyCreate(_impl);
			return signal_connector<Signature>(_impl);
		}

		Invoker invoker() const
		{
			CreationPolicy_::template LazyCreate(_impl);
			return Invoker(_impl);
		}

		void operator () (Ts... args) const
		{
			if (_impl)
				_impl->InvokeAll(args...);
		}
	};

#else

	struct ConnectionPolicy
	{
		STINGRAYKIT_ENUM_VALUES(SyncOnly, AsyncOnly, Any);
		STINGRAYKIT_DECLARE_ENUM_CLASS(ConnectionPolicy);
	};


	/**
	 * @brief Signal template
	 * @tparam Signature			The signature of the signal
	 * @tparam Strategy				The threading strategy for the signal (threaded_signal_strategy, threadless_signal_strategy)
	 * @tparam ExceptionHandler		The exception handling strategy for the signal (default_exception_handler, null_exception_handler)
	 * @tparam SendCurrentState		The populator strategy for the signal (default_send_current_state, null_send_current_state)
	 * @par Example:
	 * @code
	 * template < typename T >
	 * class MyCollection
	 * {
	 *     typedef std::vector<T>	VectorType;
	 *
	 * private:
	 *     VectorType	_vector;
	 *
	 * public:
	 *     MyCollection()
	 *         : OnChanged(Bind(&MyCollection::OnChangedPopulator, this, _1))
	 *     { }
	 *
	 *     signal<void(CollectionOp op, const T& item)>		OnChanged;
	 *
	 *     void Add(const T& item)
	 *     {
	 *         signal_locker l(OnChanged); // Locking the signal mutex that guards the state
	 *         _vector.push_back(item);
	 *         OnChanged(CollectionOp::ItemAdded, item);
	 *     }
	 *
	 *     void Remove(const T& item)
	 *     {
	 *         signal_locker l(OnChanged); // Locking the signal mutex that guards the state
	 *         VectorType::iterator it = std::find(_vector.begin(), _vector.end(), item);
	 *         if (it == _vector.end())
	 *             return;
	 *         _vector.erase(it);
	 *         OnChanged(CollectionOp::ItemRemoved, item);
	 *     }
	 *
	 * private:
	 *     void OnChangedPopulator(const function<void(CollectionOp op, const T& item)>& slot) const
	 *     {
	 *         // The signal mutex that guards the state is already locked from the outside
	 *         // Sending all the items to the slot that has just been connected
	 *         for (VectorType::const_iterator it = _vector.begin(); it != _vector.end(); ++it)
	 *             slot(CollectionOp::Added, *it);
	 *     }
	 * };
	 *
	 * // ...
	 *
	 * template < typename T >
	 * void CollectionLogger(CollectionOp op, const T& item)
	 * {
	 *     std::cout << "op: " << op.ToString() << ", item: " << item << std::endl;
	 * }
	 *
	 *
	 * void CollectionAccumulator(CollectionOp op, int item)
	 * {
	 *     statc int value = 0;
	 *     switch (op)
	 *     {
	 *     case CollectionOp::ItemAdded:
	 *         value += item;
	 *         break;
	 *     case CollectionOp::ItemRemoved:
	 *         value -= item;
	 *         break;
	 *     default:
	 *         STINGRAYKIT_THROW(NotImplementedException());
	 *     }
	 *     std::cout << "Accumulated value: " << value << std::endl;
	 * }
	 *
	 * // ...
	 *
	 * ITaskExecutorPtr async_worker = ITaskExecutor::Create("asyncWorkerThread");
	 * MyCollection<int> c;
	 * std::cout << "Connecting CollectionLogger" << std::endl;
	 * c.OnChanged.connect(&CollectionLogger);
	 * c.Add(1);
	 * c.Add(2);
	 * c.Add(3);
	 * std::cout << "Connecting CollectionAccumulator" << std::endl;
	 * c.OnChanged.connect(async_worker, &CollectionAccumulator);
	 * c.Add(4);
	 * c.Add(5);
	 *
	 * // output:
	 * // Connecting CollectionLogger
	 * // op: ItemAdded, item: 1
	 * // op: ItemAdded, item: 2
	 * // op: ItemAdded, item: 3
	 * // Connecting CollectionAccumulator
	 * // Accumulated value: 1
	 * // Accumulated value: 3
	 * // Accumulated value: 6
	 * // op: ItemAdded, item: 4
	 * // Accumulated value: 10
	 * // op: ItemAdded, item: 5
	 * // Accumulated value: 15
	 *
	 * @endcode
	 */
	template < typename Signature, typename Strategy = threaded_signal_strategy, typename ExceptionHandler = default_exception_handler, template <typename> class SendCurrentState = default_send_current_state >
	struct signal : public function_info<Signature>
	{
		/**
		 * @brief A copyable functor object that hold a reference to the signal. Be aware of possible lifetime problems!
		 */
		struct Invoker : public function_info<RetType, ParamTypes>
		{
		private:
			const signal&	_signal;

		public:
			/**
			 * @param[in] theSignal The referred signal
			 */
			Invoker(const signal& theSignal);

			/**
			 * @brief Signal invokation method
			 * @param[in] parameters The parameters that will be passed to the connected slots
			 */
			void operator () (Parameters... parameters) const;
		};

		/** @brief Constructs a signal with default exception handler, no populator, and 'Any' connection policy */
		explicit signal();

		/**
		 * @brief Constructs a signal with no populator, given exception handler, and given connection policy
		 * @param[in] sendCurrentState 'null' for the populator function
		 * @param[in] exceptionHandler The exception handler
		 * @param[in] connectionPolicy Connection policy
		 * @par Example:
		 * @code
		 * void MyExceptionHandler(const std::exception& ex)
		 * {
		 *     std::cerr << "An exception in signal handler: " << ex.what() << std::endl;
		 * }
		 * // ...
		 * signal<void(int)> some_signal(null, &MyExceptionHandler, ConnectionPolicy::Any);
		 * @endcode
		 * */
		explicit signal(NullPtrType sendCurrentState,
						const ExceptionHandlerFunc& exceptionHandler,
						ConnectionPolicy connectionPolicy = ConnectionPolicy::Any);

		/**
		 * @brief Constructs a signal with given populator, no exception handler, and given connection policy
		 * @param[in] sendCurrentState The populator function
		 * @param[in] exceptionHandler 'null' for the exception handler
		 * @param[in] connectionPolicy Connection policy
		 * @par Example:
		 * @code
		 * void PopulatorForTheSignal(const function<void(int)>& slot)
		 * {
		 *     slot(123); // The slot will receive the value of 123 when being connected to the signal
		 * }
		 * // ...
		 * signal<void(int)> some_signal(&PopulatorForTheSignal, null, ConnectionPolicy::Any);
		 * @endcode
		 * */
		explicit signal(const SendCurrentStateFunc& sendCurrentState,
						NullPtrType exceptionHandler,
						ConnectionPolicy connectionPolicy = ConnectionPolicy::Any);

		/**
		 * @brief Constructs a signal with given connection policy
		 * @param[in] connectionPolicy Connection policy
		 * @par Example:
		 * @code
		 * signal<void(int)> some_signal(ConnectionPolicy::Any);
		 * @endcode
		 * */
		explicit signal(ConnectionPolicy connectionPolicy);

		/**
		 * @brief Constructs a signal with given populator, given exception handler, and given connection policy
		 * @param[in] sendCurrentState The populator function
		 * @param[in] exceptionHandler The exception handler
		 * @param[in] connectionPolicy Connection policy
		 * @par Example:
		 * @code
		 * void MyExceptionHandler(const std::exception& ex)
		 * {
		 *     std::cerr << "An exception in signal handler: " << ex.what() << std::endl;
		 * }
		 * void PopulatorForTheSignal(const function<void(int)>& slot)
		 * {
		 *     slot(123); // The slot will receive the value of 123 when being connected to the signal
		 * }
		 * // ...
		 * signal<void(int)> some_signal(&PopulatorForTheSignal, &MyExceptionHandler, ConnectionPolicy::Any);
		 * @endcode
		 * */
		explicit signal(const SendCurrentStateFunc& sendCurrentState,
						const ExceptionHandlerFunc& exceptionHandler = &stingray::Detail::DefaultSignalExceptionHandler,
						ConnectionPolicy connectionPolicy = ConnectionPolicy::Any);

		/**
		 * @brief Signal invokation method
		 * @param[in] parameters The parameters that will be passed to the connected slots
		 */
		void operator () (Parameters... parameters) const;

		/**
		 * @brief A getter of a copyable object that may be used to construct a function object (the signal itself is not copyable).
		 * @returns A copyable functor object.
		 */
		Invoker invoker() const;

		/**
		 * @brief A getter of an object that may be used to connect to the signal.
		 * @returns A signal_connector object.
		 */
		signal_connector<Signature> connector() const;

		/**
		 * @brief Invoke the populator for a given function
		 * @param[in] connectingSlot The function that will be invoked from the populator
		 */
		void SendCurrentState(const FuncType& connectingSlot) const;

		/**
		 * @brief Asynchronous connect method. Is prohibited if the signal uses ConnectionPolicy::SyncOnly
		 * @param[in] executor The ITaskExecutor object that will be used for the handler invokation
		 * @param[in] handler The signal handler function (slot)
		 */
		Token connect(const ITaskExecutorPtr& executor, const FuncType& handler) const;

		/**
		 * @brief Synchronous connect method. Is prohibited if the signal uses ConnectionPolicy::AsyncOnly
		 * @param[in] handler The signal handler function (slot)
		 */
		Token connect(const FuncType& handler) const;

		STINGRAYKIT_NONCOPYABLE(signal);
	}

#endif

	/** @} */

}

#endif
