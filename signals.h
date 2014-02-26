#ifndef STINGRAY_TOOLKIT_SIGNALS_H
#define STINGRAY_TOOLKIT_SIGNALS_H


#include <stingray/log/Logger.h>
#include <stingray/toolkit/ITaskExecutor.h>
#include <stingray/toolkit/assert.h>
#include <stingray/toolkit/async_function.h>
#include <stingray/toolkit/function.h>
#include <stingray/toolkit/inplace_vector.h>
#include <stingray/toolkit/intrusive_list.h>
#include <stingray/toolkit/self_counter.h>
#include <stingray/toolkit/signal_connection.h>
#include <stingray/toolkit/signal_connector.h>
#include <stingray/toolkit/signal_policies.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

#ifndef DOXYGEN_PREPROCESSOR

	namespace Detail
	{

		struct FuncTypeWithDeathControl
		{
		private:
			function_storage		_func;
			FutureExecutionTester	_tester;

		public:
			FuncTypeWithDeathControl(const function_storage& func, const FutureExecutionTester& tester = null)
				: _func(func), _tester(tester)
			{ }
			const function_storage& Func() 	{ return _func; }
			FutureExecutionTester& Tester()	{ return _tester; }
		};


		template<typename Signature, typename ExceptionHandlerFunc, size_t ParamsNum = GetTypeListLength<typename function_info<Signature>::ParamTypes>::Value>
		struct ExceptionHandlerWrapper;

#define DETAIL_EXCEPTION_HANDLER_PARAM_DECL(Index_, UserArg_) TOOLKIT_COMMA_IF(Index_) typename GetParamPassingType<typename GetTypeListItem<ParamTypes, Index_>::ValueT>::ValueT p##Index_
#define DETAIL_EXCEPTION_HANDLER_PARAM_USAGE(Index_, UserArg_) TOOLKIT_COMMA_IF(Index_) p##Index_
#define DETAIL_DECLARE_EXCEPTION_HANDLER_WRAPPER(N_, UserArg_) \
		template<typename Signature, typename ExceptionHandlerFunc> \
		struct ExceptionHandlerWrapper<Signature, ExceptionHandlerFunc, N_> : public function_info<Signature> \
		{ \
			typedef function<Signature>								FuncType; \
			typedef typename function_info<Signature>::ParamTypes	ParamTypes; \
			const FuncType& _func; \
			const ExceptionHandlerFunc& _exFunc; \
			ExceptionHandlerWrapper(const FuncType& func, const ExceptionHandlerFunc& exFunc) : _func(func), _exFunc(exFunc) \
			{ } \
			void operator() (TOOLKIT_REPEAT(N_, DETAIL_EXCEPTION_HANDLER_PARAM_DECL, ~)) const \
			{ \
				try { _func(TOOLKIT_REPEAT(N_, DETAIL_EXCEPTION_HANDLER_PARAM_USAGE, ~)); } \
				catch (std::exception& ex) { _exFunc(ex); } \
			} \
		};

		TOOLKIT_REPEAT_NESTING_2(10, DETAIL_DECLARE_EXCEPTION_HANDLER_WRAPPER, ~)

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


		template < typename Signature_, typename ThreadingPolicy_, typename ExceptionPolicy_, typename PopulatorsPolicy_, typename ConnectionPolicyControl_ >
		class SignalImpl : public ThreadingPolicy_, public ExceptionPolicy_, public PopulatorsPolicy_, public ConnectionPolicyControl_, public ISignalConnector<Signature_>
		{
			template < typename Signature2_, typename ThreadingPolicy2_, typename ExceptionPolicy2_, typename PopulatorsPolicy2_, typename ConnectionPolicyControl2_, typename CreationPolicy2_ >
			friend class signal;

			template < typename Signature2_, typename ThreadingPolicy2_, typename ExceptionPolicy2_, typename PopulatorsPolicy2_, typename ConnectionPolicyControl2_ >
			friend class Connection;

			typedef SignalImpl<Signature_, ThreadingPolicy_, ExceptionPolicy_, PopulatorsPolicy_, ConnectionPolicyControl_>		MyType;
			typedef typename function_info<Signature_>::ParamTypes																ParamTypes;

			typedef function<void(const std::exception&)>			ExceptionHandlerFunc;
			typedef function<void(const function<Signature_>&)>		PopulatorFunc;

		public:
			typedef intrusive_list_node_wrapper<FuncTypeWithDeathControl>	FuncTypeWrapper;
			typedef intrusive_list<FuncTypeWrapper>							Handlers;

		private:
			Handlers		_handlers;

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

			virtual ISignalConnectionSelfCountPtr Connect(const function<Signature_>& funcStorage, const FutureExecutionTester& futureExecutionTester, const TaskLifeToken& taskLifeToken);

		private:
			template<typename ContainerType>
			void CopyHandlersToLocal(ContainerType & localCopy) const
			{
				typename ThreadingPolicy_::LockType l(this->GetSync());
				std::copy(_handlers.begin(), _handlers.end(), std::back_inserter(localCopy));
			}

			void InvokeAll(const Tuple<ParamTypes>& p) const
			{
				typedef inplace_vector<FuncTypeWithDeathControl, 16> local_copy_type;
				local_copy_type local_copy;
				this->CopyHandlersToLocal(local_copy);

				for (typename local_copy_type::iterator it = local_copy.begin(); it != local_copy.end(); ++it)
				{
					FuncTypeWithDeathControl& func = (*it);

					LocalExecutionGuard guard;
					if (func.Tester().Execute(guard))
						WRAP_EXCEPTION_HANDLING(this->GetExceptionHandler(), FunctorInvoker::Invoke(func.Func().ToFunction<Signature_>(), p); );
				}
			}
		};


		template < typename Signature_, typename ThreadingPolicy_, typename ExceptionPolicy_, typename PopulatorsPolicy_, typename ConnectionPolicyControl_ >
		class Connection : public ISignalConnection
		{
		public:
			typedef SignalImpl<Signature_, ThreadingPolicy_, ExceptionPolicy_, PopulatorsPolicy_, ConnectionPolicyControl_>		Impl;
			typedef self_count_ptr<Impl>									ImplPtr;

		private:
			typedef intrusive_list_node_wrapper<FuncTypeWithDeathControl>	FuncTypeWrapper;
			typedef intrusive_list<FuncTypeWrapper>							Handlers;
			typedef typename Handlers::iterator								IteratorType;

			ImplPtr				_signalImpl;
			IteratorType		_it;
			TaskLifeToken		_token;

		public:
			Connection(const ImplPtr& signalImpl, typename Handlers::iterator it, const TaskLifeToken& token)
				: _signalImpl(signalImpl), _it(it), _token(token)
			{ }

			virtual void Disconnect()
			{
				ImplPtr signal_impl = _signalImpl;
				if (signal_impl)
				{
					typename ThreadingPolicy_::LockType l(signal_impl->GetSync());
					_signalImpl.reset();
					Handlers &handlers = signal_impl->_handlers;
					if (_it != handlers.end())
					{
						handlers.erase(_it);
						_it = handlers.end();
					}
				}
				_token.Release();
			}
		};

		template < typename Signature_, typename ThreadingPolicy_, typename ExceptionPolicy_, typename PopulatorsPolicy_, typename ConnectionPolicyControl_ >
		ISignalConnectionSelfCountPtr SignalImpl<Signature_, ThreadingPolicy_, ExceptionPolicy_, PopulatorsPolicy_, ConnectionPolicyControl_>::Connect(const function<Signature_>& func, const FutureExecutionTester& futureExecutionTester, const TaskLifeToken& taskLifeToken)
		{
			//TOOLKIT_ASSERT(this->GetConnectionPolicy() != ConnectionPolicy::AsyncOnly);
			typedef Connection<Signature_, ThreadingPolicy_, ExceptionPolicy_, PopulatorsPolicy_, ConnectionPolicyControl_>	Connection;

			typename ThreadingPolicy_::LockType l(this->GetSync());
			Detail::ExceptionHandlerWrapper<Signature_, function<void(const std::exception&)> > wrapped_slot(func, this->GetExceptionHandler());
			WRAP_EXCEPTION_HANDLING(this->GetExceptionHandler(), this->template SendCurrentState<Signature_>(wrapped_slot); );

			_handlers.push_back(FuncTypeWrapper(FuncTypeWithDeathControl(function_storage(func), futureExecutionTester)));
			typename Connection::ImplPtr impl(this);
			this->add_ref();
			return ISignalConnectionSelfCountPtr(new Connection(impl, --_handlers.end(), taskLifeToken));
		}

	}


	class signal_locker
	{
		TOOLKIT_NONCOPYABLE(signal_locker);

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
#define DETAIL_SIGNAL_TEMPLATE_PARAM_USAGE(Index_, UserArg_) TOOLKIT_COMMA_IF(Index_) T##Index_
#define DETAIL_SIGNAL_PARAM_DECL(Index_, UserArg_) TOOLKIT_COMMA_IF(Index_) T##Index_ p##Index_
#define DETAIL_SIGNAL_PARAM_USAGE(Index_, UserArg_) TOOLKIT_COMMA_IF(Index_) p##Index_
#define DETAIL_DECLARE_SIGNAL(N_, UserArg_) \
	template < TOOLKIT_REPEAT(N_, DETAIL_SIGNAL_TEMPLATE_PARAM_DECL, ~) typename ThreadingPolicy_, typename ExceptionPolicy_, typename PopulatorsPolicy_, typename ConnectionPolicyControl_, typename CreationPolicy_ > \
	class signal<void(TOOLKIT_REPEAT(N_, DETAIL_SIGNAL_TEMPLATE_PARAM_USAGE, ~)), ThreadingPolicy_, ExceptionPolicy_, PopulatorsPolicy_, ConnectionPolicyControl_, CreationPolicy_> \
	{ \
		TOOLKIT_NONCOPYABLE(signal); \
		\
		friend class signal_locker; \
		template < typename Signature2_ > friend class signal_connector; \
		\
	public: \
		typedef void Signature(TOOLKIT_REPEAT(N_, DETAIL_SIGNAL_TEMPLATE_PARAM_USAGE, ~)); \
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
	public: \
		class Invoker : public function_info<Signature> \
		{ \
		private: \
			ImplPtr		_impl; \
		public: \
			Invoker(const ImplPtr& impl) : _impl(impl) { } \
			void operator () (TOOLKIT_REPEAT(N_, DETAIL_SIGNAL_PARAM_DECL, ~)) const \
			{ _impl->InvokeAll(Tuple<ParamTypes>(TOOLKIT_REPEAT(N_, DETAIL_SIGNAL_PARAM_USAGE, ~))); } \
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
		\
		void SendCurrentState(const function<Signature>& slot) const \
		{ \
			if (!_impl) \
				return; \
			typename ThreadingPolicy_::LockType l(_impl->GetSync()); \
			Detail::ExceptionHandlerWrapper<Signature, ExceptionHandlerFunc, GetTypeListLength<ParamTypes>::Value > wrapped_slot(slot, _impl->GetExceptionHandler()); \
			WRAP_EXCEPTION_HANDLING( _impl->GetExceptionHandler(), _impl->template SendCurrentState<Signature>(wrapped_slot); ); \
		} \
		\
		signal_connection connect(const function<Signature>& slot) const \
		{ \
			CreationPolicy_::template LazyCreate(_impl); \
			TaskLifeToken token; \
			return signal_connection(_impl->Connect(slot, token.GetExecutionTester(), token)); \
		} \
		\
		signal_connection connect(const ITaskExecutorPtr& worker, const function<Signature>& slot) const \
		{ \
			CreationPolicy_::template LazyCreate(_impl); \
			async_function<Signature> slot_func(worker, slot); \
			return signal_connection(_impl->Connect(slot_func, null, slot_func.GetToken())); \
		} \
		\
		signal_connector<Signature> connector() const { return signal_connector<Signature>(_impl); } \
		Invoker invoker() const { CreationPolicy_::template LazyCreate(_impl); return Invoker(_impl); } \
		\
		void operator () (TOOLKIT_REPEAT(N_, DETAIL_SIGNAL_PARAM_DECL, ~)) const \
		{ \
			if (!_impl) \
				return; \
			_impl->InvokeAll(Tuple<ParamTypes>(TOOLKIT_REPEAT(N_, DETAIL_SIGNAL_PARAM_USAGE, ~))); \
		} \
	};

	TOOLKIT_REPEAT_NESTING_2(10, DETAIL_DECLARE_SIGNAL, ~)

#undef DETAIL_DECLARE_SIGNAL
#undef DETAIL_SIGNAL_PARAM_USAGE
#undef DETAIL_SIGNAL_PARAM_DECL
#undef DETAIL_SIGNAL_TEMPLATE_PARAM_USAGE
#undef DETAIL_SIGNAL_TEMPLATE_PARAM_DECL

#else

	struct ConnectionPolicy
	{
		TOOLKIT_ENUM_VALUES(SyncOnly, AsyncOnly, Any);
		TOOLKIT_DECLARE_ENUM_CLASS(ConnectionPolicy);
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
	 *         TOOLKIT_THROW(NotImplementedException());
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
		signal_connection connect(const ITaskExecutorPtr& executor, const FuncType& handler) const;

		/**
		 * @brief Synchronous connect method. Is prohibited if the signal uses ConnectionPolicy::AsyncOnly
		 * @param[in] handler The signal handler function (slot)
		 */
		signal_connection connect(const FuncType& handler) const;

		TOOLKIT_NONCOPYABLE(signal);
	}

#endif

	/** @} */

}

#endif
