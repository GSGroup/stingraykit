#ifndef STINGRAY_TOOLKIT_THREADLESS_SIGNALS_H
#define STINGRAY_TOOLKIT_THREADLESS_SIGNALS_H

#include <stingray/toolkit/signals.h>
#include <stingray/toolkit/light_shared_ptr.h>
#include <stingray/toolkit/task_alive_token.h>
#include <stingray/toolkit/inplace_vector.h>

namespace stingray
{

	namespace Detail
	{

		class ThreadlessConnection : public ISignalConnection
		{
		public:
			typedef intrusive_list_node_wrapper<FuncTypeWithDeathControl>	FuncTypeWrapper;
			typedef intrusive_list<FuncTypeWrapper>							Handlers;
			typedef light_shared_ptr<Handlers>								HandlersPtr;
			typedef light_weak_ptr<Handlers>								HandlersWeakPtr;
			typedef ISignalConnection::VTable								VTable;
			typedef Handlers::iterator										IteratorType;

		private:
			HandlersWeakPtr									_handlers;
			IteratorType									_it;
			TaskLifeToken									_token;

		public:
			inline ThreadlessConnection(const HandlersWeakPtr& handlers, IteratorType it, const TaskLifeToken& token)
				: _handlers(handlers), _it(it), _token(token)
			{ _getVTable = &GetVTable; }

			static VTable GetVTable()
			{ return VTable(&_Dtor, &_Disconnect); }

			static void _Dtor(ISignalConnection *self)
			{ static_cast<ThreadlessConnection *>(self)->Dtor(); }

			static void _Disconnect(ISignalConnection *self)
			{ static_cast<ThreadlessConnection *>(self)->Disconnect(); }

			void Dtor()
			{
				_token.~TaskLifeToken();
				_handlers.~HandlersWeakPtr();
				_it.~IteratorType();
			}

			void Disconnect()
			{
				HandlersPtr handlers_l = _handlers.lock();
				if (handlers_l)
					handlers_l->erase(_it);
				_token.Release();
			}
		};

	} //namespace Detail

	template < typename Signature , typename ExceptionHandler, template <typename> class SendCurrentState_ >
	class threadless_signal_base : private ExceptionHandler, private SendCurrentState_<Signature>
	{
		TOOLKIT_NONCOPYABLE(threadless_signal_base);

		typedef function<Signature>									FuncType;

	public:
		typedef typename function_info<Signature>::RetType			RetType;
		typedef typename function_info<Signature>::ParamTypes		ParamTypes;
		typedef Tuple<ParamTypes>									TupleParamsType;

	protected:
		typedef typename ExceptionHandler::ExceptionHandlerFunc		ExceptionHandlerFunc;
		typedef SendCurrentState_<Signature>						SendCurrentStateBase;
		typedef typename SendCurrentStateBase::SendCurrentStateFunc	SendCurrentStateFunc;
		typedef typename SendCurrentStateBase::InvokeFuncType		InvokeFuncType;
		typedef void InvokeFuncSignature(const TupleParamsType &);

	private:
		typedef Detail::FuncTypeWithDeathControl						FuncTypeWithDeathControl;
		typedef intrusive_list_node_wrapper<FuncTypeWithDeathControl>	FuncTypeWrapper;
		typedef intrusive_list<FuncTypeWrapper>							Handlers;
		typedef Detail::ThreadlessConnection							Connection;

		mutable light_shared_ptr<Handlers>								_handlers;
		ConnectionPolicy												_connectionPolicy;

	protected:
		inline threadless_signal_base(const ExceptionHandlerFunc& exceptionHandler, const SendCurrentStateBase& sendCurrentState, ConnectionPolicy connectionPolicy)
			: ExceptionHandler(exceptionHandler), SendCurrentStateBase(sendCurrentState), _handlers(), _connectionPolicy(connectionPolicy)
		{ }

		inline ~threadless_signal_base() { }

		inline void InvokeAll(const TupleParamsType& p) const
		{ InvokeAll(p, this->GetExceptionHandler()); }

		void InvokeAll(const TupleParamsType& p, const ExceptionHandlerFunc& exceptionHandler) const
		{
			if (!_handlers)
				return;

			typedef inplace_vector<FuncTypeWithDeathControl, 16> local_copy_type;
			local_copy_type local_copy;
			std::copy(_handlers->begin(), _handlers->end(), std::back_inserter(local_copy));

			for (typename local_copy_type::iterator it = local_copy.begin(); it != local_copy.end(); ++it)
			{
				try
				{
					FuncTypeWithDeathControl& func = (*it);
					LocalExecutionGuard guard;
					if (func.Tester().Execute(guard))
						func.Func().ToFunction<InvokeFuncSignature>()(p);
				}
				catch (const std::exception& ex)
				{ exceptionHandler(ex); }
			}
		}

		static inline void DefaultSendCurrentState(const FuncType& /*connectingSlot*/)
		{ }

	public:

		inline function<void(const FuncType&)> GetCurrentStateSender() const
		{ return bind(&threadless_signal_base::DoSendCurrentState, this, _1); }

		inline signal_connection connect(const ITaskExecutorPtr& executor, const FuncType& handler) const
		{
			TOOLKIT_ASSERT(_connectionPolicy != ConnectionPolicy::SyncOnly);

			async_function<Signature> slot_func(executor, handler);
			Detail::FunctionArgumentWrapper<Signature, ExceptionHandlerFunc> wrapped_handler(function<Signature>(slot_func), this->GetExceptionHandler());
			this->DoSendCurrentState(wrapped_handler);
			if (!_handlers)
				_handlers.reset(new Handlers);
			_handlers->push_back(FuncTypeWrapper(FuncTypeWithDeathControl(function_storage(function<InvokeFuncSignature>(wrapped_handler)))));
			return signal_connection(Detail::ISignalConnectionSelfCountPtr(new Connection(_handlers, --_handlers->end(), slot_func.GetToken())));
		}

		inline signal_connection connect(const FuncType& handler) const
		{
			TOOLKIT_ASSERT(_connectionPolicy != ConnectionPolicy::AsyncOnly);

			Detail::FunctionArgumentWrapper<Signature, ExceptionHandlerFunc> wrapped_handler(handler, this->GetExceptionHandler());
			this->DoSendCurrentState(wrapped_handler);
			if (!_handlers)
				_handlers.reset(new Handlers);
			TaskLifeToken token;
			_handlers->push_back(FuncTypeWrapper(FuncTypeWithDeathControl(function_storage(function<InvokeFuncSignature>(wrapped_handler)), token.GetExecutionTester())));
			return signal_connection(Detail::ISignalConnectionSelfCountPtr(new Connection(_handlers, --_handlers->end(), token)));
		}
	};

	struct threadless_signal_strategy {};

	template<typename Signature, typename ExceptionHandler, template <typename> class SendCurrentState >
	struct signal_base<Signature, threadless_signal_strategy, ExceptionHandler, SendCurrentState> : public threadless_signal_base<Signature, ExceptionHandler, SendCurrentState>
	{
		typedef threadless_signal_base<Signature, ExceptionHandler, SendCurrentState> base;
		typedef typename base::ExceptionHandlerFunc ExceptionHandlerFunc;
		typedef typename base::SendCurrentStateFunc SendCurrentStateFunc;

		explicit inline signal_base(const ExceptionHandlerFunc& exceptionHandler, const typename base::SendCurrentStateBase& sendCurrentState, ConnectionPolicy connectionPolicy): base(exceptionHandler, sendCurrentState, connectionPolicy) { }
	};

	class threadless_signal_connection_pool
	{
		TOOLKIT_NONCOPYABLE(threadless_signal_connection_pool);

		typedef std::vector<signal_connection> signal_connection_list;
		signal_connection_list _connections;

	public:
		inline threadless_signal_connection_pool()	{ }
		inline ~threadless_signal_connection_pool()	{ release(); }

		inline void add(const signal_connection & conn)
		{ _connections.push_back(conn); }

		inline bool empty() const
		{ return _connections.empty(); }

		inline void release()
		{
			std::for_each(_connections.rbegin(), _connections.rend(),
				std::mem_fun_ref(&signal_connection::disconnect));
		}

		inline threadless_signal_connection_pool& operator+= (const signal_connection& conn) { add(conn); return *this; }
	};

}

#endif
