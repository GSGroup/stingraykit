#ifndef STINGRAYKIT_SIGNAL_VALUE_SIGNAL_CONNECTOR_H
#define STINGRAYKIT_SIGNAL_VALUE_SIGNAL_CONNECTOR_H

#include <stingraykit/signal/signal_connector.h>

namespace stingray
{

	namespace Detail
	{

		template < typename Signature >
		class ValueSignalConnectorImpl;

		template < typename... Ts >
		class ValueSignalConnectorImpl<void (Ts...)> final : public virtual ISignalConnector
		{
			class Connection final : public IToken
			{
			private:
				TaskLifeToken		_token;

			public:
				explicit Connection(TaskLifeToken&& token)
					: 	_token(std::move(token))
				{ }

				~Connection() override
				{ _token.Release(); }
			};

			using TupleType = Tuple<TypeList<typename Decay<Ts>::ValueT...>>;

		private:
			TupleType		_values;

		public:
			template < typename ValueTypeList >
			explicit ValueSignalConnectorImpl(Tuple<ValueTypeList>&& values)
				:	_values(std::move(values))
			{ }

			Token Connect(const function_storage& func, const FutureExecutionTester&, TaskLifeToken&& token, bool sendCurrentState) override
			{
				if (sendCurrentState)
					SendCurrentState(func);

				return MakeToken<Connection>(std::move(token));
			}

			void SendCurrentState(const function_storage& slot) const override
			{
				try
				{ FunctorInvoker::Invoke(slot.ToFunction<void (Ts...)>(), _values); }
				catch (const std::exception& ex)
				{ signal_policies::exception_handling::DefaultSignalExceptionHandler(ex); }
			}

			TaskLifeToken CreateSyncToken() const override { return TaskLifeToken::CreateDummyTaskToken(); }
			TaskLifeToken CreateAsyncToken() const override { return TaskLifeToken(); }
		};

		template < typename... Ts >
		class ValueSignalConnectorProxy
		{
			using TupleType = Tuple<TypeList<typename Decay<Ts>::ValueT...>>;

		private:
			TupleType		_values;

		public:
			explicit ValueSignalConnectorProxy(Ts&&... args)
				:	_values(std::forward<Ts>(args)...)
			{ }

			template < typename Signature, ConnectionPolicy::Enum ConnectionPolicy >
			operator signal_connector<Signature, ConnectionPolicy> () &&
			{ return signal_connector<Signature, ConnectionPolicy>(make_self_count_ptr<Detail::ValueSignalConnectorImpl<Signature>>(std::move(_values))); }
		};

	}


	template < typename... Ts >
	Detail::ValueSignalConnectorProxy<Ts...> make_value_signal_connector(Ts&&... args)
	{ return Detail::ValueSignalConnectorProxy<Ts...>(std::forward<Ts>(args)...); }

}

#endif
