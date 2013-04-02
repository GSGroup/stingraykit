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
			FORCE_INLINE ThreadlessConnection(const HandlersWeakPtr& handlers, IteratorType it, const TaskLifeToken& token)
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

	protected:
		typedef typename ExceptionHandler::ExceptionHandlerFunc		ExceptionHandlerFunc;
		typedef SendCurrentState_<Signature>						SendCurrentStateBase;
		typedef typename SendCurrentStateBase::SendCurrentStateFunc	SendCurrentStateFunc;

	private:
		typedef Detail::FuncTypeWithDeathControl						FuncTypeWithDeathControl;
		typedef intrusive_list_node_wrapper<FuncTypeWithDeathControl>	FuncTypeWrapper;
		typedef intrusive_list<FuncTypeWrapper>							Handlers;
		typedef Detail::ThreadlessConnection							Connection;

		mutable light_shared_ptr<Handlers>								_handlers;

	protected:
		FORCE_INLINE threadless_signal_base(const ExceptionHandlerFunc& exceptionHandler, const SendCurrentStateFunc& sendCurrentState)
			: ExceptionHandler(exceptionHandler), SendCurrentStateBase(sendCurrentState), _handlers()
		{ }

		FORCE_INLINE ~threadless_signal_base() { }

		FORCE_INLINE void InvokeAll(const Tuple<typename function_info<Signature>::ParamTypes>& p) const
		{ InvokeAll(p, this->GetExceptionHandler()); }

		void InvokeAll(const Tuple<typename function_info<Signature>::ParamTypes>& p, const ExceptionHandlerFunc& exceptionHandler) const
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
					ExecutionToken token;
					if (func.Token().Execute(token))
						FunctorInvoker::Invoke(func.Func().ToFunction<Signature>(), p);
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
			slot<Signature> slot_func(executor, handler);
			function<Signature> slot_function(slot_func);
			Detail::ExceptionHandlerWrapper<Signature, ExceptionHandlerFunc, GetTypeListLength<ParamTypes>::Value> wrapped_slot(slot_function, this->GetExceptionHandler());
			this->DoSendCurrentState(wrapped_slot);
			if (!_handlers)
				_handlers.reset(new Handlers);
			_handlers->push_back(FuncTypeWrapper(FuncTypeWithDeathControl(function_storage(slot_function))));
			return signal_connection(Detail::ISignalConnectionSelfCountPtr(new Connection(_handlers, --_handlers->end(), slot_func.GetToken())));
		}

		inline signal_connection connect(const FuncType& handler) const
		{
			Detail::ExceptionHandlerWrapper<Signature, ExceptionHandlerFunc, GetTypeListLength<ParamTypes>::Value> wrapped_slot(handler, this->GetExceptionHandler());
			this->DoSendCurrentState(wrapped_slot);
			if (!_handlers)
				_handlers.reset(new Handlers);
			TaskLifeToken token;
			_handlers->push_back(FuncTypeWrapper(FuncTypeWithDeathControl(function_storage(handler), token.GetExecutionToken())));
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

		explicit FORCE_INLINE signal_base(const ExceptionHandlerFunc& exceptionHandler, const SendCurrentStateFunc& sendCurrentState): base(exceptionHandler, sendCurrentState) {}
	};

}

#endif
