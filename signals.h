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

/*! \cond GS_INTERNAL */

namespace stingray
{
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
		typedef function<Signature>								FuncType;
		typedef function<void(const FuncType& connectingSlot)>	SendCurrentStateFunc;

		typedef typename function_info<Signature>::ParamTypes   ParamTypes;
		typedef function<void (const Tuple<ParamTypes>& ) >     InvokeFuncType;
		typedef function<void (const SendCurrentStateFunc&, const InvokeFuncType &)> ForwardCurrentState;

		inline null_send_current_state(const SendCurrentStateFunc &, const ForwardCurrentState &) { }
		static inline void DoSendCurrentState(const InvokeFuncType &) { }
	};

	template<typename Signature>
	struct default_send_current_state
	{
		typedef function<Signature>								FuncType;
		typedef function<void(const FuncType& connectingSlot)>	SendCurrentStateFunc;

		typedef typename function_info<Signature>::ParamTypes   ParamTypes;
		typedef function<void (const Tuple<ParamTypes>& ) >     InvokeFuncType;
		typedef function<void (const SendCurrentStateFunc&, const InvokeFuncType &)> ForwardCurrentState;

		SendCurrentStateFunc									_sendCurrentState;
		ForwardCurrentState										_forward;

		inline default_send_current_state(const SendCurrentStateFunc& func, const ForwardCurrentState &adapter) : _sendCurrentState(func), _forward(adapter) {}
		inline void DoSendCurrentState(const InvokeFuncType &func)	const	{ _forward(_sendCurrentState, func); }
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

	namespace Detail
	{
		template<typename Signature, typename ExceptionHandlerFunc>
		class FunctionArgumentWrapper : public function_info<void (const Tuple<typename function_info<Signature>::ParamTypes>& ) >
		{
		public:
			typedef function<Signature>								FuncType;
			typedef typename function_info<Signature>::RetType		RetType;
			typedef typename function_info<Signature>::ParamTypes	ParamTypes;

			const FuncType					_func;
			const ExceptionHandlerFunc		_exFunc;

		public:
			inline FunctionArgumentWrapper(const FuncType &func, const ExceptionHandlerFunc& exFunc) : _func(func), _exFunc(exFunc) {}

			inline void operator() (const Tuple<ParamTypes> &args) const
			{
				try { FunctorInvoker::Invoke(_func, args); }
				catch (std::exception& ex) { _exFunc(ex); }
			}
		};
	}

	template < typename Signature_, typename ExceptionHandler, template <typename> class SendCurrentState_ >
	class threaded_signal_base : private threaded_signal_base_base, private ExceptionHandler, private SendCurrentState_<Signature_>
	{
		TOOLKIT_NONCOPYABLE(threaded_signal_base);

		friend class signal_locker;
		typedef Signature_												Signature;
		typedef function<Signature>										FuncType;

	public:
		typedef typename function_info<Signature>::RetType				RetType;
		typedef typename function_info<Signature>::ParamTypes			ParamTypes;
		typedef Tuple<ParamTypes>										TupleParamsType;

	protected:
		typedef SendCurrentState_<Signature>							SendCurrentStateBase;
		typedef typename SendCurrentStateBase::SendCurrentStateFunc		SendCurrentStateFunc;
		typedef typename ExceptionHandler::ExceptionHandlerFunc			ExceptionHandlerFunc;

		typedef typename SendCurrentStateBase::InvokeFuncType			InvokeFuncType;
		typedef void InvokeFuncSignature(const TupleParamsType &);

	private:
		typedef threaded_signal_base_base::FuncTypeWithDeathControl		FuncTypeWithDeathControl;

	private:
		ConnectionPolicy												_connectionPolicy;

	protected:
		inline threaded_signal_base(const ExceptionHandlerFunc& exceptionHandler, const SendCurrentStateBase& sendCurrentState, ConnectionPolicy connectionPolicy)
			: ExceptionHandler(exceptionHandler), SendCurrentStateBase(sendCurrentState), _connectionPolicy(connectionPolicy)
		{ }

		inline ~threaded_signal_base() { }

		inline void InvokeAll(const TupleParamsType& p) const
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

		void InvokeAll(const TupleParamsType & p, const ExceptionHandlerFunc& exceptionHandler) const
		{
			typedef inplace_vector<FuncTypeWithDeathControl, 16> local_copy_type;
			local_copy_type local_copy;
			this->CopyHandlersToLocal(local_copy);

			for (typename local_copy_type::iterator it = local_copy.begin(); it != local_copy.end(); ++it)
			{
				FuncTypeWithDeathControl& func = (*it);

				LocalExecutionGuard guard;
				if (func.Tester().Execute(guard))
					WRAP_EXCEPTION_HANDLING( exceptionHandler, func.Func().ToFunction<InvokeFuncSignature>()(p); );
			}
			if (!_handlers)
				TOOLKIT_FATAL("Signal was destroyed while executing handlers!");
		}

		static inline void DefaultSendCurrentState(const FuncType& /*connectingSlot*/)
		{ }

	public:
		inline void SendCurrentState(const FuncType& connectingSlot) const
		{
			MutexLock l(this->_handlers->second);
			Detail::FunctionArgumentWrapper<Signature, ExceptionHandlerFunc> wrapped_slot(connectingSlot, this->GetExceptionHandler());
			WRAP_EXCEPTION_HANDLING( this->GetExceptionHandler(), this->DoSendCurrentState(wrapped_slot); );
		}

		inline function<void(const FuncType&)> GetCurrentStateSender() const
		{ return bind(&threaded_signal_base::SendCurrentState, this, _1); }


		signal_connection connect(const ITaskExecutorPtr& executor, const FuncType& handler) const
		{
			TOOLKIT_ASSERT(_connectionPolicy != ConnectionPolicy::SyncOnly);

			MutexLock l(this->_handlers->second);
			async_function<Signature> slot_func(executor, handler);
			Detail::FunctionArgumentWrapper<Signature, ExceptionHandlerFunc> wrapped_slot(slot_func, this->GetExceptionHandler());
			WRAP_EXCEPTION_HANDLING(this->GetExceptionHandler(), this->DoSendCurrentState(wrapped_slot); );
			return this->AddFunc(function_storage(function<InvokeFuncSignature>(wrapped_slot)), null, slot_func.GetToken()); // Using real execution token instead of null may cause deadlocks!!!
		}

		signal_connection connect(const FuncType& handler) const
		{
			TOOLKIT_ASSERT(_connectionPolicy != ConnectionPolicy::AsyncOnly);

			MutexLock l(this->_handlers->second);
			TaskLifeToken token;
			Detail::FunctionArgumentWrapper<Signature, ExceptionHandlerFunc> wrapped_handler(handler, this->GetExceptionHandler());
			WRAP_EXCEPTION_HANDLING(this->GetExceptionHandler(), this->DoSendCurrentState(wrapped_handler); );
			return this->AddFunc(function_storage(function<InvokeFuncSignature>(wrapped_handler)), token.GetExecutionTester(), token);
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
		typedef typename base::SendCurrentStateBase		SendCurrentStateBase;

		explicit inline signal_base(const ExceptionHandlerFunc& exceptionHandler, const SendCurrentStateBase& sendCurrentState, ConnectionPolicy connectionPolicy): base(exceptionHandler, sendCurrentState, connectionPolicy) { }
	};

	template < typename Signature, typename Strategy = threaded_signal_strategy, typename ExceptionHandler = default_exception_handler, template <typename> class SendCurrentState = default_send_current_state >
	struct signal;

	template <typename Strategy, typename ExceptionHandler, template <typename> class SendCurrentState >
	struct signal<void(), Strategy, ExceptionHandler, SendCurrentState> : public signal_base<void(), Strategy, ExceptionHandler, SendCurrentState>
	{
		typedef signal_base<void(), Strategy, ExceptionHandler, SendCurrentState> base;
		typedef	typename base::ExceptionHandlerFunc		ExceptionHandlerFunc;
		typedef	typename base::SendCurrentStateFunc		SendCurrentStateFunc;
		typedef typename base::SendCurrentStateBase		SendCurrentStateBase;

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

		static inline void ForwardCurrentStateAdapter(const typename base::InvokeFuncType &sendStateTuple)
		{ sendStateTuple(Tuple<TypeList_0>()); }

		static inline void ForwardCurrentState(const SendCurrentStateFunc& sendStateArgs, const typename base::InvokeFuncType &sendStateTuple)
		{
			sendStateArgs(bind(&signal::ForwardCurrentStateAdapter, sendStateTuple));
		}

		explicit inline signal(const NullPtrType&,
									 const ExceptionHandlerFunc& exceptionHandler,
									 ConnectionPolicy connectionPolicy = ConnectionPolicy::Any)
			: base(exceptionHandler, SendCurrentStateBase(&base::DefaultSendCurrentState, &signal::ForwardCurrentState), connectionPolicy)
		{ }

		explicit inline signal(const SendCurrentStateFunc& sendCurrentState,
									 const NullPtrType&,
									 ConnectionPolicy connectionPolicy = ConnectionPolicy::Any)
			: base(&stingray::Detail::DefaultSignalExceptionHandler, SendCurrentStateBase(sendCurrentState, &signal::ForwardCurrentState), connectionPolicy)
		{ }

		explicit inline signal(ConnectionPolicy connectionPolicy)
			: base(&stingray::Detail::DefaultSignalExceptionHandler, SendCurrentStateBase(&base::DefaultSendCurrentState, &signal::ForwardCurrentState), connectionPolicy)
		{ }

		explicit inline signal(const SendCurrentStateFunc& sendCurrentState = &base::DefaultSendCurrentState,
									 const ExceptionHandlerFunc& exceptionHandler = &stingray::Detail::DefaultSignalExceptionHandler,
									 ConnectionPolicy connectionPolicy = ConnectionPolicy::Any)
			: base(exceptionHandler, SendCurrentStateBase(sendCurrentState, &signal::ForwardCurrentState), connectionPolicy)
		{ }

		inline void operator () () const
		{
			this->InvokeAll(Tuple<TypeList_0>());
		}

		inline void call(const ExceptionHandlerFunc& exceptionHandler) const
		{
			this->InvokeAll(Tuple<TypeList_0>(), exceptionHandler);
		}

		CopyableRef copyable_ref() const { return CopyableRef(*this); }

		TOOLKIT_NONCOPYABLE(signal);
	};


#define DETAIL_TOOLKIT_DECLARE_SIGNAL(Typenames_, Types_, Decl_, Usage_, Placeholders_) \
	template < Typenames_, typename Strategy , typename ExceptionHandler, template <typename> class SendCurrentState > \
	struct signal<void(Types_), Strategy, ExceptionHandler, SendCurrentState> : public signal_base<void(Types_), Strategy, ExceptionHandler, SendCurrentState> \
	{ \
		typedef signal_base<void(Types_), Strategy, ExceptionHandler, SendCurrentState>		base; \
		typedef	typename base::ExceptionHandlerFunc		ExceptionHandlerFunc; \
		typedef	typename base::SendCurrentStateFunc		SendCurrentStateFunc; \
		typedef typename base::SendCurrentStateBase		SendCurrentStateBase; \
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
		static inline void ForwardCurrentStateAdapter(const typename base::InvokeFuncType &sendStateTuple, Decl_) \
		{ sendStateTuple(PackArguments(Usage_)); } \
		\
		static inline void ForwardCurrentState(const SendCurrentStateFunc& sendStateArgs, const typename base::InvokeFuncType &sendStateTuple) { \
			sendStateArgs(bind(&signal::ForwardCurrentStateAdapter, sendStateTuple, Placeholders_)); \
		} \
		\
		explicit inline signal(const NullPtrType&, \
									 const ExceptionHandlerFunc& exceptionHandler, \
									 ConnectionPolicy connectionPolicy = ConnectionPolicy::Any) \
			: base(exceptionHandler, SendCurrentStateBase(&base::DefaultSendCurrentState, &signal::ForwardCurrentState), connectionPolicy) \
		{ } \
		\
		explicit inline signal(const SendCurrentStateFunc& sendCurrentState, \
									 const NullPtrType&, \
									 ConnectionPolicy connectionPolicy = ConnectionPolicy::Any) \
			: base(&stingray::Detail::DefaultSignalExceptionHandler, SendCurrentStateBase(sendCurrentState, &signal::ForwardCurrentState), connectionPolicy) \
		{ } \
		\
		explicit inline signal(ConnectionPolicy connectionPolicy) \
			: base(&stingray::Detail::DefaultSignalExceptionHandler, SendCurrentStateBase(&base::DefaultSendCurrentState, &signal::ForwardCurrentState), connectionPolicy) \
		{ } \
		\
		explicit inline signal(const SendCurrentStateFunc& sendCurrentState = &base::DefaultSendCurrentState, \
									 const ExceptionHandlerFunc& exceptionHandler = &stingray::Detail::DefaultSignalExceptionHandler, \
									 ConnectionPolicy connectionPolicy = ConnectionPolicy::Any) \
			: base(exceptionHandler, SendCurrentStateBase(sendCurrentState, &signal::ForwardCurrentState), connectionPolicy) \
		{ } \
		\
		static inline Tuple<TYPELIST(Types_)> PackArguments(Decl_) \
		{ return Tuple<TYPELIST(Types_)>(Usage_); } \
		\
		inline void operator () (Decl_) const \
		{ this->InvokeAll(PackArguments(Usage_)); } \
		\
		inline void call(Decl_, const ExceptionHandlerFunc& exceptionHandler) const \
		{ this->InvokeAll(PackArguments(Usage_), exceptionHandler); } \
		\
		CopyableRef copyable_ref() const { return CopyableRef(*this); } \
		\
		TOOLKIT_NONCOPYABLE(signal); \
	}

#define TY typename
#define P_(N) typename GetParamPassingType<T##N>::ValueT p##N
#define B_(N) _##N

	DETAIL_TOOLKIT_DECLARE_SIGNAL(MK_PARAM(TY T1), MK_PARAM(T1), MK_PARAM(P_(1)), MK_PARAM(p1), MK_PARAM(B_(1)));
	DETAIL_TOOLKIT_DECLARE_SIGNAL(MK_PARAM(TY T1, TY T2), MK_PARAM(T1, T2), MK_PARAM(P_(1), P_(2)), MK_PARAM(p1, p2), MK_PARAM(B_(1), B_(2)));
	DETAIL_TOOLKIT_DECLARE_SIGNAL(MK_PARAM(TY T1, TY T2, TY T3), MK_PARAM(T1, T2, T3), MK_PARAM(P_(1), P_(2), P_(3)), MK_PARAM(p1, p2, p3), MK_PARAM(B_(1), B_(2), B_(3)));
	DETAIL_TOOLKIT_DECLARE_SIGNAL(MK_PARAM(TY T1, TY T2, TY T3, TY T4), MK_PARAM(T1, T2, T3, T4), MK_PARAM(P_(1), P_(2), P_(3), P_(4)), MK_PARAM(p1, p2, p3, p4), MK_PARAM(B_(1), B_(2), B_(3), B_(4)));
	DETAIL_TOOLKIT_DECLARE_SIGNAL(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5), MK_PARAM(T1, T2, T3, T4, T5), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5)), MK_PARAM(p1, p2, p3, p4, p5), MK_PARAM(B_(1), B_(2), B_(3), B_(4), B_(5)));
	DETAIL_TOOLKIT_DECLARE_SIGNAL(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6), MK_PARAM(T1, T2, T3, T4, T5, T6), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6)), MK_PARAM(p1, p2, p3, p4, p5, p6), MK_PARAM(B_(1), B_(2), B_(3), B_(4), B_(5), B_(6)));
	DETAIL_TOOLKIT_DECLARE_SIGNAL(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7), MK_PARAM(T1, T2, T3, T4, T5, T6, T7), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7), MK_PARAM(B_(1), B_(2), B_(3), B_(4), B_(5), B_(6), B_(7)));
	DETAIL_TOOLKIT_DECLARE_SIGNAL(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8), MK_PARAM(B_(1), B_(2), B_(3), B_(4), B_(5), B_(6), B_(7), B_(8)));
	DETAIL_TOOLKIT_DECLARE_SIGNAL(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9), MK_PARAM(B_(1), B_(2), B_(3), B_(4), B_(5), B_(6), B_(7), B_(8), B_(9)));
	DETAIL_TOOLKIT_DECLARE_SIGNAL(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10), MK_PARAM(B_(1), B_(2), B_(3), B_(4), B_(5), B_(6), B_(7), B_(8), B_(9), B_(10)));

#undef B_
#undef P_
#undef TY

}

/*! \endcond */


#endif
