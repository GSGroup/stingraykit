#ifndef STINGRAY_TOOLKIT_SIGNALS_H
#define STINGRAY_TOOLKIT_SIGNALS_H

#include <stingray/toolkit/assert.h>
#include <stingray/toolkit/signal_connection.h>
#include <stingray/toolkit/function.h>
#include <stingray/toolkit/async_function.h>
#include <stingray/toolkit/intrusive_list.h>
#include <stingray/toolkit/inplace_vector.h>
#include <stingray/threads/Thread.h>
#include <stingray/log/Logger.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

	struct default_exception_handler
	{
		typedef function<void(const std::exception&)>	ExceptionHandlerFunc;
		ExceptionHandlerFunc							_exceptionHandler;
		default_exception_handler(const ExceptionHandlerFunc &ehf) : _exceptionHandler(ehf) {}
		const ExceptionHandlerFunc& GetExceptionHandler() const	{ return _exceptionHandler; }
	};

	struct null_exception_handler
	{
		typedef function<void(const std::exception&)>	ExceptionHandlerFunc;
		null_exception_handler(const ExceptionHandlerFunc &ehf) {}
		static void handle(const std::exception &ex)	{ Logger::Error() << "Uncaught exception in signal handler: " << diagnostic_information(ex); }
		static ExceptionHandlerFunc GetExceptionHandler()	{ return &null_exception_handler::handle; }
	};

	template<typename Signature>
	struct null_send_current_state
	{
	private:
		struct Dummy {};
	public:
		typedef function<Signature>								FuncType;
		typedef function<void(const FuncType& connectingSlot)>	SendCurrentStateFunc;
		null_send_current_state()									{}
		null_send_current_state(const SendCurrentStateFunc &)		{ CompileTimeAssert<SameType<Dummy, Signature>::Value> ERROR_this_populator_will_not_work_with_null_strategy; }
		static void DoSendCurrentState(const FuncType &func)		{ }
	};

	template<typename Signature>
	struct default_send_current_state
	{
		typedef function<Signature>								FuncType;
		typedef function<void(const FuncType& connectingSlot)>	SendCurrentStateFunc;
		SendCurrentStateFunc									_sendCurrentState;

		static inline void DefaultSendCurrentState(const FuncType& /*connectingSlot*/)
		{ }

		default_send_current_state() : _sendCurrentState(&DefaultSendCurrentState) {}
		default_send_current_state(const SendCurrentStateFunc& func) : _sendCurrentState(func) {}
		void DoSendCurrentState(const FuncType &func)	const		{ _sendCurrentState(func); }
	};


	class signal_locker
	{
		TOOLKIT_NONCOPYABLE(signal_locker);

	private:
		MutexLock	_mutexLock;

	public:
		template < typename SignalType >
		signal_locker(const SignalType& theSignal)
			: _mutexLock(theSignal.GetSyncRoot())
		{ }
	};

#ifndef DOXYGEN_PREPROCESSOR

	namespace Detail
	{
		template<typename Signature, typename ExceptionHandlerFunc, size_t ParamsNum>
		struct ExceptionHandlerWrapper;

		template<typename Signature, typename ExceptionHandlerFunc>
		struct ExceptionHandlerWrapper<Signature, ExceptionHandlerFunc, 0> : public function_info<Signature>
		{
			typedef function<Signature>					FuncType;

			const FuncType& _func;
			const ExceptionHandlerFunc& _exFunc;

			ExceptionHandlerWrapper(const FuncType& func, const ExceptionHandlerFunc& exFunc) : _func(func), _exFunc(exFunc)
			{}

			void operator() () const
			{
				try
				{ _func(); }
				catch (std::exception& ex)
				{ _exFunc(ex); }
			}
		};

#define P_(N) typename GetParamPassingType<typename GetTypeListItem<ParamTypes, N - 1>::ValueT>::ValueT p##N
#define DETAIL_DECLARE_EXCEPTION_HANDLER_WRAPPER(ParamsNum_, ParamsDecl_, ParamsUsage_) \
		template<typename Signature, typename ExceptionHandlerFunc> \
		struct ExceptionHandlerWrapper<Signature, ExceptionHandlerFunc, ParamsNum_> : public function_info<Signature> \
		{ \
			typedef function<Signature>								FuncType; \
			typedef typename function_info<Signature>::ParamTypes	ParamTypes; \
			const FuncType& _func; \
			const ExceptionHandlerFunc& _exFunc; \
			ExceptionHandlerWrapper(const FuncType& func, const ExceptionHandlerFunc& exFunc) : _func(func), _exFunc(exFunc) \
			{} \
			void operator() (ParamsDecl_) const \
			{ \
				try \
				{ _func(ParamsUsage_); } \
				catch (std::exception& ex) \
				{ _exFunc(ex); } \
			} \
		};

		DETAIL_DECLARE_EXCEPTION_HANDLER_WRAPPER(1, MK_PARAM(P_(1)), MK_PARAM(p1))
		DETAIL_DECLARE_EXCEPTION_HANDLER_WRAPPER(2, MK_PARAM(P_(1), P_(2)), MK_PARAM(p1, p2))
		DETAIL_DECLARE_EXCEPTION_HANDLER_WRAPPER(3, MK_PARAM(P_(1), P_(2), P_(3)), MK_PARAM(p1, p2, p3))
		DETAIL_DECLARE_EXCEPTION_HANDLER_WRAPPER(4, MK_PARAM(P_(1), P_(2), P_(3), P_(4)), MK_PARAM(p1, p2, p3, p4))
		DETAIL_DECLARE_EXCEPTION_HANDLER_WRAPPER(5, MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5)), MK_PARAM(p1, p2, p3, p4, p5))
		DETAIL_DECLARE_EXCEPTION_HANDLER_WRAPPER(6, MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6)), MK_PARAM(p1, p2, p3, p4, p5, p6))
		DETAIL_DECLARE_EXCEPTION_HANDLER_WRAPPER(7, MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7))
		DETAIL_DECLARE_EXCEPTION_HANDLER_WRAPPER(8, MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8))
		DETAIL_DECLARE_EXCEPTION_HANDLER_WRAPPER(9, MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9))
		DETAIL_DECLARE_EXCEPTION_HANDLER_WRAPPER(10,MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10))

#undef DETAIL_DECLARE_EXCEPTION_HANDLER_WRAPPER
#undef P_
	 }


	class threaded_signal_base_base
	{
	protected:
		typedef Detail::FuncTypeWithDeathControl						FuncTypeWithDeathControl;
		typedef intrusive_list_node_wrapper<FuncTypeWithDeathControl>	FuncTypeWrapper;
		typedef intrusive_list<FuncTypeWrapper>							Handlers;
		typedef Detail::ThreadedConnection<Handlers>					Connection;
		typedef Connection::HandlersType								HandlersType;

		shared_ptr<HandlersType>										_handlers;

		inline threaded_signal_base_base()
			: _handlers(new std::pair<Handlers, Mutex>)
		{ }

		~threaded_signal_base_base()
		{ _handlers.reset(); }

		template<typename ContainerType>
		void CopyHandlersToLocal(ContainerType & localCopy) const
		{
			MutexLock l(_handlers->second);
			std::copy(_handlers->first.begin(), _handlers->first.end(), std::back_inserter(localCopy));
		}

		signal_connection AddFunc(const function_storage& funcStorage, const FutureExecutionTester& futureExecutionTester, const TaskLifeToken& taskLifeToken) const
		{
			_handlers->first.push_back(FuncTypeWrapper(FuncTypeWithDeathControl(funcStorage, futureExecutionTester)));
			return signal_connection(Detail::ISignalConnectionSelfCountPtr(new Connection(_handlers, --_handlers->first.end(), taskLifeToken)));
		}
	};


	struct ConnectionPolicy
	{
		TOOLKIT_ENUM_VALUES(SyncOnly, AsyncOnly, Any);
		TOOLKIT_DECLARE_ENUM_CLASS(ConnectionPolicy);
	};


	template < typename Signature, typename ExceptionHandler, template <typename> class SendCurrentState_ >
	class threaded_signal_base : private threaded_signal_base_base, private ExceptionHandler, private SendCurrentState_<Signature>
	{
		TOOLKIT_NONCOPYABLE(threaded_signal_base);

		friend class signal_locker;
		typedef function<Signature>										FuncType;

	public:
		typedef typename function_info<Signature>::RetType				RetType;
		typedef typename function_info<Signature>::ParamTypes			ParamTypes;

	protected:
		typedef SendCurrentState_<Signature>							SendCurrentStateBase;
		typedef typename SendCurrentStateBase::SendCurrentStateFunc		SendCurrentStateFunc;
		typedef typename ExceptionHandler::ExceptionHandlerFunc			ExceptionHandlerFunc;

	private:
		typedef threaded_signal_base_base::FuncTypeWithDeathControl		FuncTypeWithDeathControl;

	private:
		ConnectionPolicy												_connectionPolicy;

	protected:
		inline threaded_signal_base(const ExceptionHandlerFunc& exceptionHandler, const SendCurrentStateFunc& sendCurrentState, ConnectionPolicy connectionPolicy)
			: ExceptionHandler(exceptionHandler), SendCurrentStateBase(sendCurrentState), _connectionPolicy(connectionPolicy)
		{ }

		inline threaded_signal_base(const ExceptionHandlerFunc& exceptionHandler, ConnectionPolicy connectionPolicy)
			: ExceptionHandler(exceptionHandler), _connectionPolicy(connectionPolicy)
		{ }

		inline ~threaded_signal_base() { }

		inline void InvokeAll(const Tuple<ParamTypes>& p) const
		{
			InvokeAll(p, this->GetExceptionHandler());
		}

#define WRAP_EXCEPTION_HANDLING(exceptionHandler, ...) \
		try \
		{ __VA_ARGS__ } \
		catch (const std::exception& ex) \
		{ \
			try { exceptionHandler(ex); } catch(const std::exception &exex) { Logger::Error() << "Exception inside exception handler: " << diagnostic_information(exex); } \
		}

		void InvokeAll(const Tuple<ParamTypes>& p, const ExceptionHandlerFunc& exceptionHandler) const
		{
			typedef inplace_vector<FuncTypeWithDeathControl, 16> local_copy_type;
			local_copy_type local_copy;
			this->CopyHandlersToLocal(local_copy);

			for (typename local_copy_type::iterator it = local_copy.begin(); it != local_copy.end(); ++it)
			{
				FuncTypeWithDeathControl& func = (*it);

				LocalExecutionGuard guard;
				if (func.Tester().Execute(guard))
					WRAP_EXCEPTION_HANDLING( exceptionHandler, FunctorInvoker::Invoke(func.Func().ToFunction<Signature>(), p); );
			}
			if (!_handlers)
				TOOLKIT_FATAL("Signal was destroyed while executing handlers!");
		}

	public:
		inline void SendCurrentState(const FuncType& connectingSlot) const
		{
			MutexLock l(this->_handlers->second);
			Detail::ExceptionHandlerWrapper<Signature, ExceptionHandlerFunc, GetTypeListLength<ParamTypes>::Value > wrapped_slot(connectingSlot, this->GetExceptionHandler());
			WRAP_EXCEPTION_HANDLING( this->GetExceptionHandler(), this->DoSendCurrentState(wrapped_slot); );
		}

		inline function<void(const FuncType&)> GetCurrentStateSender() const
		{ return bind(&threaded_signal_base::SendCurrentState, this, _1); }


		signal_connection connect(const ITaskExecutorPtr& executor, const FuncType& handler) const
		{
			TOOLKIT_ASSERT(_connectionPolicy != ConnectionPolicy::SyncOnly);

			MutexLock l(this->_handlers->second);
			async_function<Signature> slot_func(executor, handler);
			function<Signature> slot_function(slot_func);
			Detail::ExceptionHandlerWrapper<Signature, ExceptionHandlerFunc, GetTypeListLength<ParamTypes>::Value> wrapped_slot(slot_function, this->GetExceptionHandler());
			WRAP_EXCEPTION_HANDLING(this->GetExceptionHandler(), this->DoSendCurrentState(wrapped_slot); );
			return this->AddFunc(function_storage(slot_function), null, slot_func.GetToken()); // Using real execution token instead of null may cause deadlocks!!!
		}

		signal_connection connect(const FuncType& handler) const
		{
			TOOLKIT_ASSERT(_connectionPolicy != ConnectionPolicy::AsyncOnly);

			MutexLock l(this->_handlers->second);
			Detail::ExceptionHandlerWrapper<Signature, ExceptionHandlerFunc, GetTypeListLength<ParamTypes>::Value> wrapped_slot(handler, this->GetExceptionHandler());
			WRAP_EXCEPTION_HANDLING(this->GetExceptionHandler(), this->DoSendCurrentState(wrapped_slot); );
			TaskLifeToken token;
			return this->AddFunc(function_storage(handler), token.GetExecutionTester(), token);
		}

#undef WRAP_EXCEPTION_HANDLING

	private:
		const Mutex& GetSyncRoot() const { return this->_handlers->second; }
	};

	struct threaded_signal_strategy {};

	template<typename Signature, typename Strategy, typename ExceptionHandler, template <typename> class SendCurrentState >
	struct signal_base;

	template<typename Signature, typename ExceptionHandler, template <typename> class SendCurrentState >
	struct signal_base<Signature, threaded_signal_strategy, ExceptionHandler, SendCurrentState> : public threaded_signal_base<Signature, ExceptionHandler, SendCurrentState>
	{
		typedef threaded_signal_base<Signature, ExceptionHandler, SendCurrentState> base;
		typedef typename base::ExceptionHandlerFunc ExceptionHandlerFunc;
		typedef typename base::SendCurrentStateFunc SendCurrentStateFunc;

		inline signal_base(const ExceptionHandlerFunc& exceptionHandler, ConnectionPolicy connectionPolicy): base(exceptionHandler, connectionPolicy) { }
		inline signal_base(const ExceptionHandlerFunc& exceptionHandler, const SendCurrentStateFunc& sendCurrentState, ConnectionPolicy connectionPolicy): base(exceptionHandler, sendCurrentState, connectionPolicy) { }
	};

	template < typename Signature, typename Strategy = threaded_signal_strategy, typename ExceptionHandler = default_exception_handler, template <typename> class SendCurrentState = default_send_current_state >
	struct signal;

	template <typename Strategy, typename ExceptionHandler, template <typename> class SendCurrentState >
	struct signal<void(), Strategy, ExceptionHandler, SendCurrentState> : public signal_base<void(), Strategy, ExceptionHandler, SendCurrentState>
	{
		typedef signal_base<void(), Strategy, ExceptionHandler, SendCurrentState> base;
		typedef	typename base::ExceptionHandlerFunc		ExceptionHandlerFunc;
		typedef	typename base::SendCurrentStateFunc		SendCurrentStateFunc;
		typedef void Signature();

		struct CopyableRef
		{
			typedef typename base::RetType		RetType;
			typedef typename base::ParamTypes	ParamTypes;

		private:
			const signal&	_signal;

		public:
			inline CopyableRef(const signal& theSignal) : _signal(theSignal) { }
			inline void operator () () const
			{ _signal(); }
		};
		explicit inline signal(): base(&stingray::Detail::DefaultSignalExceptionHandler, ConnectionPolicy::Any) {}

		explicit inline signal(const NullPtrType&,
									 const ExceptionHandlerFunc& exceptionHandler,
									 ConnectionPolicy connectionPolicy = ConnectionPolicy::Any)
			: base(exceptionHandler, &base::DefaultSendCurrentState, connectionPolicy)
		{ }

		explicit inline signal(const SendCurrentStateFunc& sendCurrentState,
									 const NullPtrType&,
									 ConnectionPolicy connectionPolicy = ConnectionPolicy::Any)
			: base(&stingray::Detail::DefaultSignalExceptionHandler, sendCurrentState, connectionPolicy)
		{ }

		explicit inline signal(ConnectionPolicy connectionPolicy)
			: base(&stingray::Detail::DefaultSignalExceptionHandler, connectionPolicy)
		{ }

		explicit inline signal(const SendCurrentStateFunc& sendCurrentState,
									 const ExceptionHandlerFunc& exceptionHandler = &stingray::Detail::DefaultSignalExceptionHandler,
									 ConnectionPolicy connectionPolicy = ConnectionPolicy::Any)
			: base(exceptionHandler, sendCurrentState, connectionPolicy)
		{ }

		inline void operator () () const
		{
			Tuple<TypeList_0> p;
			this->InvokeAll(p);
		}

		inline void call(const ExceptionHandlerFunc& exceptionHandler) const
		{
			Tuple<TypeList_0> p;
			this->InvokeAll(p, exceptionHandler);
		}

		CopyableRef copyable_ref() const { return CopyableRef(*this); }

		TOOLKIT_NONCOPYABLE(signal);
	};


#define DETAIL_TOOLKIT_DECLARE_SIGNAL(Typenames_, Types_, Decl_, Usage_) \
	template < Typenames_, typename Strategy , typename ExceptionHandler, template <typename> class SendCurrentState > \
	struct signal<void(Types_), Strategy, ExceptionHandler, SendCurrentState> : public signal_base<void(Types_), Strategy, ExceptionHandler, SendCurrentState> \
	{ \
		typedef signal_base<void(Types_), Strategy, ExceptionHandler, SendCurrentState>		base; \
		typedef void Signature(Types_); \
		typedef	typename base::ExceptionHandlerFunc		ExceptionHandlerFunc; \
		typedef	typename base::SendCurrentStateFunc		SendCurrentStateFunc; \
		\
		struct CopyableRef \
		{ \
			typedef typename signal::RetType		RetType; \
			typedef typename signal::ParamTypes	ParamTypes; \
			\
		private: \
			const signal&	_signal; \
			\
		public: \
			inline CopyableRef(const signal& theSignal) : _signal(theSignal) { } \
			inline void operator () (Decl_) const \
			{ _signal(Usage_); } \
		}; \
		\
		explicit inline signal(): base(&stingray::Detail::DefaultSignalExceptionHandler, ConnectionPolicy::Any) {} \
		explicit inline signal(const NullPtrType&, \
									 const ExceptionHandlerFunc& exceptionHandler, \
									 ConnectionPolicy connectionPolicy = ConnectionPolicy::Any) \
			: base(exceptionHandler, connectionPolicy) \
		{ } \
		\
		explicit inline signal(const SendCurrentStateFunc& sendCurrentState, \
									 const NullPtrType&, \
									 ConnectionPolicy connectionPolicy = ConnectionPolicy::Any) \
			: base(&stingray::Detail::DefaultSignalExceptionHandler, sendCurrentState, connectionPolicy) \
		{ } \
		\
		explicit inline signal(ConnectionPolicy connectionPolicy) \
			: base(&stingray::Detail::DefaultSignalExceptionHandler, connectionPolicy) \
		{ } \
		\
		explicit inline signal(const SendCurrentStateFunc& sendCurrentState, \
									 const ExceptionHandlerFunc& exceptionHandler = &stingray::Detail::DefaultSignalExceptionHandler, \
									 ConnectionPolicy connectionPolicy = ConnectionPolicy::Any) \
			: base(exceptionHandler, sendCurrentState, connectionPolicy) \
		{ } \
		\
		inline void operator () (Decl_) const \
		{ \
			Tuple<TYPELIST(Types_)> p(Usage_); \
			this->InvokeAll(p); \
		} \
		\
		inline void call(Decl_, const ExceptionHandlerFunc& exceptionHandler) const \
		{ \
			Tuple<TYPELIST(Types_)> p(Usage_); \
			this->InvokeAll(p, exceptionHandler); \
		} \
		\
		CopyableRef copyable_ref() const { return CopyableRef(*this); } \
		\
		TOOLKIT_NONCOPYABLE(signal); \
	}

#define TY typename
#define P_(N) typename GetParamPassingType<T##N>::ValueT p##N

	DETAIL_TOOLKIT_DECLARE_SIGNAL(MK_PARAM(TY T1), MK_PARAM(T1), MK_PARAM(P_(1)), MK_PARAM(p1));
	DETAIL_TOOLKIT_DECLARE_SIGNAL(MK_PARAM(TY T1, TY T2), MK_PARAM(T1, T2), MK_PARAM(P_(1), P_(2)), MK_PARAM(p1, p2));
	DETAIL_TOOLKIT_DECLARE_SIGNAL(MK_PARAM(TY T1, TY T2, TY T3), MK_PARAM(T1, T2, T3), MK_PARAM(P_(1), P_(2), P_(3)), MK_PARAM(p1, p2, p3));
	DETAIL_TOOLKIT_DECLARE_SIGNAL(MK_PARAM(TY T1, TY T2, TY T3, TY T4), MK_PARAM(T1, T2, T3, T4), MK_PARAM(P_(1), P_(2), P_(3), P_(4)), MK_PARAM(p1, p2, p3, p4));
	DETAIL_TOOLKIT_DECLARE_SIGNAL(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5), MK_PARAM(T1, T2, T3, T4, T5), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5)), MK_PARAM(p1, p2, p3, p4, p5));
	DETAIL_TOOLKIT_DECLARE_SIGNAL(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6), MK_PARAM(T1, T2, T3, T4, T5, T6), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6)), MK_PARAM(p1, p2, p3, p4, p5, p6));
	DETAIL_TOOLKIT_DECLARE_SIGNAL(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7), MK_PARAM(T1, T2, T3, T4, T5, T6, T7), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7));
	DETAIL_TOOLKIT_DECLARE_SIGNAL(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8));
	DETAIL_TOOLKIT_DECLARE_SIGNAL(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9));
	DETAIL_TOOLKIT_DECLARE_SIGNAL(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10));

#undef P_
#undef TY

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
	 */
	template < typename Signature, typename Strategy = threaded_signal_strategy, typename ExceptionHandler = default_exception_handler, template <typename> class SendCurrentState = default_send_current_state >
	struct signal : public function_info<Signature>
	{
		/**
		 * @brief A copyable functor object that hold a reference to the signal. Be aware of possible lifetime problems!
		 */
		struct CopyableRef : public function_info<RetType, ParamTypes>
		{
		private:
			const signal&	_signal;

		public:
			/**
			 * @param[in] theSignal The referred signal
			 */
			inline CopyableRef(const signal& theSignal);

			/**
			 * @brief Signal invokation method
			 * @param[in] parameters The parameters that will be passed to the connected slots
			 */
			inline void operator () (Parameters... parameters) const;
		};

		/** @brief Constructs a signal with default exception handler, no populator, and 'Any' connection policy */
		explicit inline signal();

		/**
		 * @brief Constructs a signal with no populator, given exception handler, and given connection policy
		 * @param[in] sendCurrentState 'null' for the populator function
		 * @param[in] exceptionHandler The exception handler
		 * @param[in] connectionPolicy Connection policy
		 * @par Example:
		 * @code
		 * void MyExceptionHandler(const std::exception& ex)
		 * {
		 *     std::cerr << "An exception in signal handler: " << ex.what();
		 * }
		 * // ...
		 * signal<void(int)> some_signal(null, &MyExceptionHandler, ConnectionPolicy::Any);
		 * @endcode
		 * */
		explicit inline signal(const NullPtrType& sendCurrentState,
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
		explicit inline signal(const SendCurrentStateFunc& sendCurrentState,
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
		explicit inline signal(ConnectionPolicy connectionPolicy);

		/**
		 * @brief Constructs a signal with given populator, given exception handler, and given connection policy
		 * @param[in] sendCurrentState The populator function
		 * @param[in] exceptionHandler The exception handler
		 * @param[in] connectionPolicy Connection policy
		 * @par Example:
		 * @code
		 * void MyExceptionHandler(const std::exception& ex)
		 * {
		 *     std::cerr << "An exception in signal handler: " << ex.what();
		 * }
		 * void PopulatorForTheSignal(const function<void(int)>& slot)
		 * {
		 *     slot(123); // The slot will receive the value of 123 when being connected to the signal
		 * }
		 * // ...
		 * signal<void(int)> some_signal(&PopulatorForTheSignal, &MyExceptionHandler, ConnectionPolicy::Any);
		 * @endcode
		 * */
		explicit inline signal(const SendCurrentStateFunc& sendCurrentState,
									 const ExceptionHandlerFunc& exceptionHandler = &stingray::Detail::DefaultSignalExceptionHandler,
									 ConnectionPolicy connectionPolicy = ConnectionPolicy::Any);

		/**
		 * @brief Signal invokation method
		 * @param[in] parameters The parameters that will be passed to the connected slots
		 */
		inline void operator () (Parameters... parameters) const;

		/**
		 * @brief A getter of a copyable object that may be used to construct a function object (the signal itself is not copyable).
		 * @returns A copyable functor object that hold a reference to the signal. Be aware of possible lifetime problems!
		 */
		CopyableRef copyable_ref() const { return CopyableRef(*this); }

		/**
		 * @brief Invoke the populator for a given function
		 * @param[in] connectingSlot The function that will be invoked from the populator
		 */
		inline void SendCurrentState(const FuncType& connectingSlot) const
		{
			MutexLock l(this->_handlers->second);
			Detail::ExceptionHandlerWrapper<Signature, ExceptionHandlerFunc, GetTypeListLength<ParamTypes>::Value > wrapped_slot(connectingSlot, this->GetExceptionHandler());
			WRAP_EXCEPTION_HANDLING( this->GetExceptionHandler(), this->DoSendCurrentState(wrapped_slot); );
		}

		/**
		 * @brief Get the populator function
		 * @returns The populator function wrapper (uses the signal's exception handling)
		 */
		inline function<void(const FuncType&)> GetCurrentStateSender() const
		{ return bind(&threaded_signal_base::SendCurrentState, this, _1); }

		/**
		 * @brief Asynchronous connect method. Is prohibited if the signal uses ConnectionPolicy::SyncOnly
		 * @param[in] executor The ITaskExecutor object that will be used for the handler invokation
		 * @param[in] handler The signal handler function (slot)
		 */
		signal_connection connect(const ITaskExecutorPtr& executor, const FuncType& handler) const
		{
			TOOLKIT_ASSERT(_connectionPolicy != ConnectionPolicy::SyncOnly);

			MutexLock l(this->_handlers->second);
			async_function<Signature> slot_func(executor, handler);
			function<Signature> slot_function(slot_func);
			Detail::ExceptionHandlerWrapper<Signature, ExceptionHandlerFunc, GetTypeListLength<ParamTypes>::Value> wrapped_slot(slot_function, this->GetExceptionHandler());
			WRAP_EXCEPTION_HANDLING(this->GetExceptionHandler(), this->DoSendCurrentState(wrapped_slot); );
			return this->AddFunc(function_storage(slot_function), null, slot_func.GetToken()); // Using real execution token instead of null may cause deadlocks!!!
		}

		/**
		 * @brief Synchronous connect method. Is prohibited if the signal uses ConnectionPolicy::AsyncOnly
		 * @param[in] handler The signal handler function (slot)
		 */
		signal_connection connect(const FuncType& handler) const
		{
			TOOLKIT_ASSERT(_connectionPolicy != ConnectionPolicy::AsyncOnly);

			MutexLock l(this->_handlers->second);
			Detail::ExceptionHandlerWrapper<Signature, ExceptionHandlerFunc, GetTypeListLength<ParamTypes>::Value> wrapped_slot(handler, this->GetExceptionHandler());
			WRAP_EXCEPTION_HANDLING(this->GetExceptionHandler(), this->DoSendCurrentState(wrapped_slot); );
			TaskLifeToken token;
			return this->AddFunc(function_storage(handler), token.GetExecutionTester(), token);
		}

		TOOLKIT_NONCOPYABLE(signal);
	}

#endif

	/** @} */

}



#endif
