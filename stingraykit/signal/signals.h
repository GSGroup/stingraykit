#ifndef STINGRAYKIT_SIGNAL_SIGNALS_H
#define STINGRAYKIT_SIGNAL_SIGNALS_H

#include <stingraykit/IToken.h>
#include <stingraykit/assert.h>
#include <stingraykit/collection/IntrusiveList.h>
#include <stingraykit/collection/inplace_vector.h>
#include <stingraykit/function/AsyncFunction.h>
#include <stingraykit/function/function.h>
#include <stingraykit/self_counter.h>
#include <stingraykit/signal/signal_connector.h>
#include <stingraykit/signal/signal_policies.h>
#include <stingraykit/thread/ITaskExecutor.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

#ifndef DOXYGEN_PREPROCESSOR

	namespace Detail
	{

		struct CancellableStorage
		{
		private:
			function_storage		_functionStorage;
			FutureExecutionTester	_tester;

		public:
			CancellableStorage(const function_storage& func, const FutureExecutionTester& tester) :
				_functionStorage(func), _tester(tester)
			{ }

			template <typename Signature_, typename Params_>
			void Invoke(const Params_& p) const
			{
				LocalExecutionGuard guard(_tester);
				if (guard)
					FunctorInvoker::Invoke(_functionStorage.ToFunction<Signature_>(), p);
			}
		};


		struct ThreadlessStorage
		{
		private:
			function_storage		_functionStorage;

		public:
			ThreadlessStorage(const function_storage& func, const FutureExecutionTester& tester) :
				_functionStorage(func)
			{ STINGRAYKIT_CHECK(tester.IsDummy(), "ThreadlessStorage can't be used with real tokens!"); }

			template <typename Signature_, typename Params_>
			void Invoke(const Params_& p) const
			{ FunctorInvoker::Invoke(_functionStorage.ToFunction<Signature_>(), p); }
		};


		template<typename Signature, typename ExceptionHandlerFunc, size_t ParamsNum = GetTypeListLength<typename function_info<Signature>::ParamTypes>::Value>
		struct ExceptionHandlerWrapper;

#define DETAIL_EXCEPTION_HANDLER_PARAM_DECL(Index_, UserArg_) STINGRAYKIT_COMMA_IF(Index_) typename GetParamPassingType<typename GetTypeListItem<ParamTypes, Index_>::ValueT>::ValueT p##Index_
#define DETAIL_EXCEPTION_HANDLER_PARAM_USAGE(Index_, UserArg_) STINGRAYKIT_COMMA_IF(Index_) p##Index_
#define DETAIL_DECLARE_EXCEPTION_HANDLER_WRAPPER(N_, UserArg_) \
		template<typename Signature, typename ExceptionHandlerFunc> \
		struct ExceptionHandlerWrapper<Signature, ExceptionHandlerFunc, N_> : public function_info<Signature> \
		{ \
			typedef function<Signature>								FuncType; \
			typedef typename function_info<Signature>::ParamTypes	ParamTypes; \
			\
		private: \
			FuncType				_func; \
			ExceptionHandlerFunc	_exFunc; \
			\
		public: \
			ExceptionHandlerWrapper(const FuncType& func, const ExceptionHandlerFunc& exFunc) : _func(func), _exFunc(exFunc) \
			{ } \
			void operator() (STINGRAYKIT_REPEAT(N_, DETAIL_EXCEPTION_HANDLER_PARAM_DECL, ~)) const \
			{ \
				try { _func(STINGRAYKIT_REPEAT(N_, DETAIL_EXCEPTION_HANDLER_PARAM_USAGE, ~)); } \
				catch (std::exception& ex) { _exFunc(ex); } \
			} \
		};

		STINGRAYKIT_REPEAT_NESTING_2(10, DETAIL_DECLARE_EXCEPTION_HANDLER_WRAPPER, ~)

#undef DETAIL_DECLARE_EXCEPTION_HANDLER_WRAPPER
#undef DETAIL_EXCEPTION_HANDLER_PARAM_USAGE
#undef DETAIL_EXCEPTION_HANDLER_PARAM_DECL

#define WRAP_EXCEPTION_HANDLING(exceptionHandler, ...) \
		try \
		{ __VA_ARGS__ } \
		catch (const std::exception& ex) \
		{ \
			exceptionHandler(ex); \
		}


		template <bool IsThreadsafe>
		struct SignalImplBase : public ISignalConnector
		{
			typedef typename If<IsThreadsafe, CancellableStorage, ThreadlessStorage>::ValueT	FuncType;
			typedef IntrusiveListNode<FuncType>													Handler;
			typedef IntrusiveList<FuncType>														Handlers;

		protected:
			typedef signal_policies::threading::DummyMutex										DummyMutex;
			typedef signal_policies::threading::DummyLock										DummyLock;
			typedef typename If<IsThreadsafe, const Mutex&, DummyMutex>::ValueT					MutexRefType;
			typedef typename If<IsThreadsafe, MutexLock, DummyLock>::ValueT						LockType;
			typedef inplace_vector<FuncType, 16>												LocalHandlersCopy;

		protected:
			Handlers	_handlers;

		public:
			virtual TaskLifeToken CreateSyncToken() const	{ return IsThreadsafe ? TaskLifeToken() : TaskLifeToken::CreateDummyTaskToken(); }
			virtual TaskLifeToken CreateAsyncToken() const	{ return TaskLifeToken(); }

			virtual Token Connect(const function_storage& func, const FutureExecutionTester& invokeToken, const TaskLifeToken& connectionToken, bool sendCurrentState);

			virtual void SendCurrentState(const function_storage& slot) const
			{
				LockType l(DoGetSync());
				DoSendCurrentState(slot);
			}

			void AddHandler(Handler& handler)
			{
				// mutex is locked in Connect
				_handlers.push_back(handler);
			}

			void RemoveHandler(Handler& handler)
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


		template <bool IsThreadsafe>
		class Connection : public IToken
		{
		public:
			typedef SignalImplBase<IsThreadsafe>	Impl;
			typedef self_count_ptr<Impl>			ImplPtr;
			typedef typename Impl::FuncType			FuncType;
			typedef typename Impl::Handler			Handler;

		private:
			ImplPtr				_signalImpl;
			Handler				_handler;
			TaskLifeToken		_token;

		public:
			Connection(const ImplPtr& signalImpl, const function_storage& func, const FutureExecutionTester& invokeToken, const TaskLifeToken& connectionToken) :
				_signalImpl(signalImpl), _handler(FuncType(func, invokeToken)), _token(connectionToken)
			{ _signalImpl->AddHandler(_handler); }

			virtual ~Connection()
			{
				_signalImpl->RemoveHandler(_handler);
				_token.Release();
			}
		};


		template < bool IsThreadsafe >
		Token SignalImplBase<IsThreadsafe>::Connect(const function_storage& func, const FutureExecutionTester& invokeToken, const TaskLifeToken& connectionToken, bool sendCurrentState)
		{
			LockType l(DoGetSync());
			if (sendCurrentState)
				DoSendCurrentState(func);

			typedef Connection<IsThreadsafe> Connection;
			typename Connection::ImplPtr impl(this);
			this->add_ref();

			return MakeToken<Connection>(impl, func, invokeToken, connectionToken);
		}


		template < typename Signature_, typename ThreadingPolicy_, typename ExceptionPolicy_, typename PopulatorsPolicy_, typename ConnectionPolicyControl_ >
		class SignalImpl : public ThreadingPolicy_, public ExceptionPolicy_, public PopulatorsPolicy_, public ConnectionPolicyControl_, public SignalImplBase<ThreadingPolicy_::IsThreadsafe>
		{
			template < typename Signature2_, typename ThreadingPolicy2_, typename ExceptionPolicy2_, typename PopulatorsPolicy2_, typename ConnectionPolicyControl2_, typename CreationPolicy2_ >
			friend class signal;

			typedef SignalImplBase<ThreadingPolicy_::IsThreadsafe>	base;
			typedef typename function_info<Signature_>::ParamTypes	ParamTypes;

			typedef function<void(const std::exception&)>			ExceptionHandlerFunc;
			typedef function<void(const function<Signature_>&)>		PopulatorFunc;

		public:
			SignalImpl() { }

			explicit SignalImpl(ConnectionPolicy connectionPolicy)
				: ConnectionPolicyControl_(connectionPolicy)
			{ }

			SignalImpl(const NullPtrType&, const ExceptionHandlerFunc& exceptionHandler)
				: ExceptionPolicy_(exceptionHandler)
			{ }

			SignalImpl(const NullPtrType&, const ExceptionHandlerFunc& exceptionHandler, ConnectionPolicy connectionPolicy)
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

			SignalImpl(const ThreadingPolicy_& threadingPolicy, const PopulatorFunc& sendCurrentState)
				: ThreadingPolicy_(threadingPolicy), PopulatorsPolicy_(sendCurrentState)
			{ }

			void InvokeAll(const Tuple<ParamTypes>& p) const
			{
				typename base::LocalHandlersCopy local_copy;
				{
					typename base::LockType l(this->GetSync());
					this->CopyHandlersToLocal(local_copy);
				}

				for (typename base::LocalHandlersCopy::iterator it = local_copy.begin(); it != local_copy.end(); ++it)
					WRAP_EXCEPTION_HANDLING(this->GetExceptionHandler(), it->template Invoke<Signature_>(p); );
			}

		private:
			virtual typename base::MutexRefType DoGetSync() const
			{ return this->GetSync(); }

			virtual void DoSendCurrentState(const function_storage& storage) const
			{
				Detail::ExceptionHandlerWrapper<Signature_, ExceptionHandlerFunc> wrapped_slot(storage.ToFunction<Signature_>(), this->GetExceptionHandler());
				WRAP_EXCEPTION_HANDLING(this->GetExceptionHandler(), PopulatorsPolicy_::template SendCurrentStateImpl<Signature_>(wrapped_slot); );
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

#define DETAIL_SIGNAL_TEMPLATE_PARAM_DECL(Index_, UserArg_) typename T##Index_,
#define DETAIL_SIGNAL_TEMPLATE_PARAM_USAGE(Index_, UserArg_) STINGRAYKIT_COMMA_IF(Index_) T##Index_
#define DETAIL_SIGNAL_PARAM_DECL(Index_, UserArg_) STINGRAYKIT_COMMA_IF(Index_) T##Index_ p##Index_
#define DETAIL_SIGNAL_PARAM_USAGE(Index_, UserArg_) STINGRAYKIT_COMMA_IF(Index_) p##Index_
#define DETAIL_DECLARE_SIGNAL(N_, UserArg_) \
	template < STINGRAYKIT_REPEAT(N_, DETAIL_SIGNAL_TEMPLATE_PARAM_DECL, ~) typename ThreadingPolicy_, typename ExceptionPolicy_, typename PopulatorsPolicy_, typename ConnectionPolicyControl_, typename CreationPolicy_ > \
	class signal<void(STINGRAYKIT_REPEAT(N_, DETAIL_SIGNAL_TEMPLATE_PARAM_USAGE, ~)), ThreadingPolicy_, ExceptionPolicy_, PopulatorsPolicy_, ConnectionPolicyControl_, CreationPolicy_> \
	{ \
		STINGRAYKIT_NONCOPYABLE(signal); \
		\
		friend class signal_locker; \
		template < typename Signature2_ > friend class signal_connector; \
		\
	public: \
		typedef void Signature(STINGRAYKIT_REPEAT(N_, DETAIL_SIGNAL_TEMPLATE_PARAM_USAGE, ~)); \
		\
		typedef void											RetType; \
		typedef typename function_info<Signature>::ParamTypes	ParamTypes; \
		\
	private: \
		typedef Detail::SignalImpl<Signature, ThreadingPolicy_, ExceptionPolicy_, PopulatorsPolicy_, ConnectionPolicyControl_>	Impl; \
		typedef self_count_ptr<Impl>																							ImplPtr; \
		\
		typedef function<void(const std::exception&)>			ExceptionHandlerFunc; \
		typedef function<void(const function<Signature>&)>		PopulatorFunc; \
		\
		typedef ThreadingPolicy_								ThreadingPolicy; \
		\
	public: \
		class Invoker : public function_info<Signature> \
		{ \
		private: \
			ImplPtr		_impl; \
		public: \
			Invoker(const ImplPtr& impl) : _impl(impl) { } \
			void operator () (STINGRAYKIT_REPEAT(N_, DETAIL_SIGNAL_PARAM_DECL, ~)) const \
			{ _impl->InvokeAll(Tuple<ParamTypes>(STINGRAYKIT_REPEAT(N_, DETAIL_SIGNAL_PARAM_USAGE, ~))); } \
		}; \
		\
	private: \
		mutable ImplPtr		_impl; \
		\
	public: \
		signal() : _impl(CreationPolicy_::template CtorCreate<Impl>()) { } \
		explicit signal(ConnectionPolicy connectionPolicy) : _impl(new Impl(connectionPolicy)) { } \
		signal(const NullPtrType&, const ExceptionHandlerFunc& exceptionHandler) : _impl(new Impl(null, exceptionHandler)) { } \
		signal(const NullPtrType&, const ExceptionHandlerFunc& exceptionHandler, ConnectionPolicy connectionPolicy) : _impl(new Impl(null, exceptionHandler, connectionPolicy)) { } \
		signal(const PopulatorFunc& sendCurrentState) : _impl(new Impl(sendCurrentState)) { } \
		signal(const PopulatorFunc& sendCurrentState, const NullPtrType&) : _impl(new Impl(sendCurrentState)) { } \
		signal(const PopulatorFunc& sendCurrentState, const NullPtrType&, ConnectionPolicy connectionPolicy) : _impl(new Impl(sendCurrentState, connectionPolicy)) { } \
		signal(const PopulatorFunc& sendCurrentState, const ExceptionHandlerFunc& exceptionHandler) : _impl(new Impl(sendCurrentState, exceptionHandler)) { } \
		signal(const PopulatorFunc& sendCurrentState, const ExceptionHandlerFunc& exceptionHandler, ConnectionPolicy connectionPolicy) : _impl(new Impl(sendCurrentState, exceptionHandler, connectionPolicy)) { } \
		signal(const ThreadingPolicy& threadingPolicy) : _impl(new Impl(threadingPolicy)) { } \
		signal(const ThreadingPolicy& threadingPolicy, const PopulatorFunc& sendCurrentState) : _impl(new Impl(threadingPolicy, sendCurrentState)) { } \
		\
		void SendCurrentState(const function<Signature>& slot) const \
		{ \
			if (!_impl) \
				return; \
			_impl->SendCurrentState(function_storage(slot)); \
		} \
		\
		Token connect(const function<Signature>& slot, bool sendCurrentState = true) const \
		{ \
			CreationPolicy_::template LazyCreate(_impl); \
			TaskLifeToken token(_impl->CreateSyncToken()); \
			return _impl->Connect(function_storage(slot), token.GetExecutionTester(), token, sendCurrentState); \
		} \
		\
		Token connect(const ITaskExecutorPtr& worker, const function<Signature>& slot, bool sendCurrentState = true) const \
		{ \
			CreationPolicy_::template LazyCreate(_impl); \
			TaskLifeToken token(_impl->CreateAsyncToken()); \
			return _impl->Connect(function_storage(function<Signature>(MakeAsyncFunction(worker, slot, token))), null, token, sendCurrentState); \
		} \
		\
		signal_connector<Signature> connector() const { CreationPolicy_::template LazyCreate(_impl); return signal_connector<Signature>(_impl); } \
		Invoker invoker() const { CreationPolicy_::template LazyCreate(_impl); return Invoker(_impl); } \
		\
		void operator () (STINGRAYKIT_REPEAT(N_, DETAIL_SIGNAL_PARAM_DECL, ~)) const \
		{ \
			if (!_impl) \
				return; \
			_impl->InvokeAll(Tuple<ParamTypes>(STINGRAYKIT_REPEAT(N_, DETAIL_SIGNAL_PARAM_USAGE, ~))); \
		} \
	};

	STINGRAYKIT_REPEAT_NESTING_2(10, DETAIL_DECLARE_SIGNAL, ~)

#undef DETAIL_DECLARE_SIGNAL
#undef DETAIL_SIGNAL_PARAM_USAGE
#undef DETAIL_SIGNAL_PARAM_DECL
#undef DETAIL_SIGNAL_TEMPLATE_PARAM_USAGE
#undef DETAIL_SIGNAL_TEMPLATE_PARAM_DECL

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
	 *         : OnChanged(bind(&MyCollection::OnChangedPopulator, this, _1))
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
		explicit signal(const NullPtrType& sendCurrentState,
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
						const NullPtrType& exceptionHandler,
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
