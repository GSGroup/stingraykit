#ifndef GS_DVRLIB_TOOLKIT_THREADLESS_SIGNALS_H__
#define GS_DVRLIB_TOOLKIT_THREADLESS_SIGNALS_H__

#include <stingray/toolkit/signals.h>
#include <stingray/toolkit/light_shared_ptr.h>
#include <vector>

namespace stingray
{

	namespace Detail
	{

		template<typename Handlers, typename FuncType>
		class ThreadlessConnection : public ISignalConnection
		{
		public:
			typedef Handlers							HandlersType;
			typedef light_shared_ptr<HandlersType>		HandlersPtr;
			typedef light_weak_ptr<HandlersType>		HandlersWeakPtr;
			typedef typename ISignalConnection::VTable	VTable;

		private:
			HandlersWeakPtr									_handlers;
			typedef typename Handlers::iterator				IteratorType;
			IteratorType									_it;

		public:
			FORCE_INLINE ThreadlessConnection(const HandlersWeakPtr& handlers, typename Handlers::iterator it)
				: _handlers(handlers), _it(it)
			{ _getVTable = &GetVTable; }

			static VTable GetVTable()
			{ return VTable(&_Dtor, &_Disconnect); }

			static void _Dtor(ISignalConnection *self)
			{ static_cast<ThreadlessConnection *>(self)->Dtor(); }

			static void _Disconnect(ISignalConnection *self)
			{ static_cast<ThreadlessConnection *>(self)->Disconnect(); }

			void Dtor()
			{
				_handlers.~HandlersWeakPtr();
				_it.~IteratorType();
			}

			void Disconnect()
			{
				HandlersPtr handlers_l = _handlers.lock();
				if (handlers_l)
					handlers_l->erase(_it);
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
		typedef intrusive_list_node_wrapper<FuncType>				FuncTypeWrapper;
		typedef intrusive_list<FuncTypeWrapper>						Handlers;
		typedef Detail::ThreadlessConnection<Handlers, FuncType>	Connection;
		typedef typename Connection::HandlersType					HandlersType;

		mutable light_shared_ptr<HandlersType>							_handlers;

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

			std::vector<FuncType> handlers;
			handlers.reserve(_handlers->size());
			std::copy(_handlers->begin(), _handlers->end(), std::back_inserter(handlers));

			typename std::vector<FuncType>::iterator it = handlers.begin();
			for (; it != handlers.end(); ++it)
			{
				const FuncType& func = (*it);
				try
				{ FunctorInvoker::Invoke(static_cast<const function<Signature>&>(func), p); }
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
			FuncType slot_f(slot<Signature>(executor, handler));
			Detail::ExceptionHandlerWrapper<Signature, ExceptionHandlerFunc, GetTypeListLength<ParamTypes>::Value> wrapped_slot(slot_f, this->GetExceptionHandler());
			this->DoSendCurrentState(wrapped_slot);
			if (!_handlers)
				_handlers.reset(new Handlers);
			_handlers->push_back(FuncTypeWrapper(slot_f));
			return signal_connection(Detail::ISignalConnectionSelfCountPtr(new Connection(_handlers, --_handlers->end())));
		}

		inline signal_connection connect(const FuncType& handler) const
		{
			Detail::ExceptionHandlerWrapper<Signature, ExceptionHandlerFunc, GetTypeListLength<ParamTypes>::Value> wrapped_slot(handler, this->GetExceptionHandler());
			this->DoSendCurrentState(wrapped_slot);
			if (!_handlers)
				_handlers.reset(new Handlers);
			_handlers->push_back(FuncTypeWrapper(handler));
			return signal_connection(Detail::ISignalConnectionSelfCountPtr(new Connection(_handlers, --_handlers->end())));
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
