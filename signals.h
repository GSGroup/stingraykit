#ifndef STINGRAY_TOOLKIT_SIGNALS_H
#define STINGRAY_TOOLKIT_SIGNALS_H

#include <stingray/toolkit/signal_connection.h>
#include <stingray/toolkit/function.h>
#include <stingray/toolkit/slot.h>
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
		null_send_current_state(const SendCurrentStateFunc &)		{ }
		static void DoSendCurrentState(const FuncType &func)		{ }
	};

	template<typename Signature>
	struct default_send_current_state
	{
		typedef function<Signature>								FuncType;
		typedef function<void(const FuncType& connectingSlot)>	SendCurrentStateFunc;
		SendCurrentStateFunc									_sendCurrentState;
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

#define TY typename
#define DETAIL_DECLARE_EXCEPTION_HANDLER_WRAPPER(ParamsNum_, ParamsTypes_, ParamsDecl_, ParamsUsage_) \
		template<typename Signature, typename ExceptionHandlerFunc> \
		struct ExceptionHandlerWrapper<Signature, ExceptionHandlerFunc, ParamsNum_> : public function_info<Signature> \
		{ \
			typedef function<Signature>					FuncType; \
			const FuncType& _func; \
			const ExceptionHandlerFunc& _exFunc; \
			ExceptionHandlerWrapper(const FuncType& func, const ExceptionHandlerFunc& exFunc) : _func(func), _exFunc(exFunc) \
			{} \
			template<ParamsTypes_> \
			void operator() (ParamsDecl_) const \
			{ \
				try \
				{ _func(ParamsUsage_); } \
				catch (std::exception& ex) \
				{ _exFunc(ex); } \
			} \
		};

		DETAIL_DECLARE_EXCEPTION_HANDLER_WRAPPER(1, MK_PARAM(TY T1), MK_PARAM(T1 p1), MK_PARAM(p1))
		DETAIL_DECLARE_EXCEPTION_HANDLER_WRAPPER(2, MK_PARAM(TY T1, TY T2), MK_PARAM(T1 p1, T2 p2), MK_PARAM(p1, p2))
		DETAIL_DECLARE_EXCEPTION_HANDLER_WRAPPER(3, MK_PARAM(TY T1, TY T2, TY T3), MK_PARAM(T1 p1, T2 p2, T3 p3), MK_PARAM(p1, p2, p3))
		DETAIL_DECLARE_EXCEPTION_HANDLER_WRAPPER(4, MK_PARAM(TY T1, TY T2, TY T3, TY T4), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4), MK_PARAM(p1, p2, p3, p4))
		DETAIL_DECLARE_EXCEPTION_HANDLER_WRAPPER(5, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5), MK_PARAM(p1, p2, p3, p4, p5))
		DETAIL_DECLARE_EXCEPTION_HANDLER_WRAPPER(6, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6), MK_PARAM(p1, p2, p3, p4, p5, p6))
		DETAIL_DECLARE_EXCEPTION_HANDLER_WRAPPER(7, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7), MK_PARAM(p1, p2, p3, p4, p5, p6, p7))
		DETAIL_DECLARE_EXCEPTION_HANDLER_WRAPPER(8, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8))
		DETAIL_DECLARE_EXCEPTION_HANDLER_WRAPPER(9, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8, T9 p9), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9))
		DETAIL_DECLARE_EXCEPTION_HANDLER_WRAPPER(10, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8, T9 p9, T10 p10), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10))

#undef DETAIL_DECLARE_EXCEPTION_HANDLER_WRAPPER
#undef TY
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

		FORCE_INLINE threaded_signal_base_base()
			: _handlers(new std::pair<Handlers, Mutex>)
		{ }

		~threaded_signal_base_base()
		{ }

		template<typename ContainerType>
		void CopyHandlersToLocal(ContainerType & localCopy) const
		{
			MutexLock l(_handlers->second);
			std::copy(_handlers->first.begin(), _handlers->first.end(), std::back_inserter(localCopy));
		}

		signal_connection AddFunc(const function_storage& funcStorage, const FutureExecutionToken& futureExecutionToken, const TaskLifeToken& taskLifeToken) const
		{
			_handlers->first.push_back(FuncTypeWrapper(FuncTypeWithDeathControl(funcStorage, futureExecutionToken)));
			return signal_connection(Detail::ISignalConnectionSelfCountPtr(new Connection(_handlers, --_handlers->first.end(), taskLifeToken)));
		}
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

	protected:
		FORCE_INLINE threaded_signal_base(const ExceptionHandlerFunc& exceptionHandler, const SendCurrentStateFunc& sendCurrentState)
			: ExceptionHandler(exceptionHandler), SendCurrentStateBase(sendCurrentState)
		{ }

		FORCE_INLINE ~threaded_signal_base() { }

		FORCE_INLINE void InvokeAll(const Tuple<typename function_info<Signature>::ParamTypes>& p) const
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

		void InvokeAll(const Tuple<typename function_info<Signature>::ParamTypes>& p, const ExceptionHandlerFunc& exceptionHandler) const
		{
			typedef inplace_vector<FuncTypeWithDeathControl, 16> local_copy_type;
			local_copy_type local_copy;
			this->CopyHandlersToLocal(local_copy);

			for (typename local_copy_type::iterator it = local_copy.begin(); it != local_copy.end(); ++it)
			{
				FuncTypeWithDeathControl& func = (*it);

				ExecutionToken token;
				if (func.Token().Execute(token))
					WRAP_EXCEPTION_HANDLING( exceptionHandler, FunctorInvoker::Invoke(func.Func().ToFunction<Signature>(), p); );
			}
		}

		static inline void DefaultSendCurrentState(const FuncType& /*connectingSlot*/)
		{ }

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
			MutexLock l(this->_handlers->second);
			slot<Signature> slot_func(executor, handler);
			function<Signature> slot_function(slot_func);
			Detail::ExceptionHandlerWrapper<Signature, ExceptionHandlerFunc, GetTypeListLength<ParamTypes>::Value> wrapped_slot(slot_function, this->GetExceptionHandler());
			WRAP_EXCEPTION_HANDLING(this->GetExceptionHandler(), this->DoSendCurrentState(wrapped_slot); );
			return this->AddFunc(function_storage(slot_function), slot_func.GetToken().GetExecutionToken(), slot_func.GetToken());
		}

		signal_connection connect(const FuncType& handler) const
		{
			MutexLock l(this->_handlers->second);
			Detail::ExceptionHandlerWrapper<Signature, ExceptionHandlerFunc, GetTypeListLength<ParamTypes>::Value> wrapped_slot(handler, this->GetExceptionHandler());
			WRAP_EXCEPTION_HANDLING(this->GetExceptionHandler(), this->DoSendCurrentState(wrapped_slot); );
			TaskLifeToken token;
			return this->AddFunc(function_storage(handler), token.GetExecutionToken(), token);
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

		explicit FORCE_INLINE signal_base(const ExceptionHandlerFunc& exceptionHandler, const SendCurrentStateFunc& sendCurrentState): base(exceptionHandler, sendCurrentState) {}
	};

	template < typename Signature, typename Strategy = threaded_signal_strategy, typename ExceptionHandler = default_exception_handler, template <typename> class SendCurrentState = default_send_current_state >
	struct signal;

	template <typename Strategy, typename ExceptionHandler, template <typename> class SendCurrentState >
	struct signal<void(), Strategy, ExceptionHandler, SendCurrentState> : public signal_base<void(), Strategy, ExceptionHandler, SendCurrentState>
	{
		typedef signal_base<void(), Strategy, ExceptionHandler, SendCurrentState> base;
		typedef	typename base::ExceptionHandlerFunc		ExceptionHandlerFunc;
		typedef	typename base::SendCurrentStateFunc		SendCurrentStateFunc;

		struct CopyableRef
		{
			typedef typename base::RetType		RetType;
			typedef typename base::ParamTypes	ParamTypes;

		private:
			const signal&	_signal;

		public:
			FORCE_INLINE CopyableRef(const signal& theSignal) : _signal(theSignal) { }
			FORCE_INLINE void operator () () const
			{ _signal(); }
		};

		explicit FORCE_INLINE signal(const NullPtrType&,
									 const ExceptionHandlerFunc& exceptionHandler = &stingray::Detail::DefaultSignalExceptionHandler)
			: base(exceptionHandler, &base::DefaultSendCurrentState)
		{ }

		explicit FORCE_INLINE signal(const SendCurrentStateFunc& sendCurrentState = &base::DefaultSendCurrentState,
									 const ExceptionHandlerFunc& exceptionHandler = &stingray::Detail::DefaultSignalExceptionHandler)
			: base(exceptionHandler, sendCurrentState)
		{}

		FORCE_INLINE void operator () () const
		{
			Tuple<TypeList_0> p;
			this->InvokeAll(p);
		}

		FORCE_INLINE void call(const ExceptionHandlerFunc& exceptionHandler) const
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
			FORCE_INLINE CopyableRef(const signal& theSignal) : _signal(theSignal) { } \
			FORCE_INLINE void operator () (Decl_) const \
			{ _signal(Usage_); } \
		}; \
		\
		explicit FORCE_INLINE signal(const NullPtrType&, \
									 const ExceptionHandlerFunc& exceptionHandler = &stingray::Detail::DefaultSignalExceptionHandler) \
			: base(exceptionHandler, &base::DefaultSendCurrentState) \
		{ } \
		\
		explicit FORCE_INLINE signal(const SendCurrentStateFunc& sendCurrentState = &base::DefaultSendCurrentState, \
									 const ExceptionHandlerFunc& exceptionHandler = &stingray::Detail::DefaultSignalExceptionHandler) \
			: base(exceptionHandler, sendCurrentState) \
		{ } \
		\
		FORCE_INLINE void operator () (Decl_) const \
		{ \
			Tuple<TYPELIST(Types_)> p(Usage_); \
			this->InvokeAll(p); \
		} \
		\
		FORCE_INLINE void call(Decl_, const ExceptionHandlerFunc& exceptionHandler) const \
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
#define P_(N) typename GetConstReferenceType<T##N>::ValueT p##N

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

}

/*! \endcond */


#endif
