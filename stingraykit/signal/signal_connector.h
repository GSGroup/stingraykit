#ifndef STINGRAYKIT_SIGNAL_SIGNAL_CONNECTOR_H
#define STINGRAYKIT_SIGNAL_SIGNAL_CONNECTOR_H


#include <stingraykit/IToken.h>
#include <stingraykit/function/AsyncFunction.h>
#include <stingraykit/task_alive_token.h>
#include <stingraykit/thread/ITaskExecutor.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

	namespace Detail
	{

		struct ISignalConnector : public self_counter<ISignalConnector>
		{
			virtual ~ISignalConnector() { }

			virtual Token Connect(const function_storage& func, const FutureExecutionTester& invokeToken, const TaskLifeToken& connectionToken, bool sendCurrentState) = 0;
			virtual void SendCurrentState(const function_storage& slot) const = 0;

			virtual TaskLifeToken CreateSyncToken() const = 0;
			virtual TaskLifeToken CreateAsyncToken() const = 0;
		};

	}

	template < typename Signature_ >
	class signal_connector
	{
	private:
		self_count_ptr<Detail::ISignalConnector>	_impl;

	public:
		signal_connector(const self_count_ptr<Detail::ISignalConnector>& impl) : _impl(impl)
		{ }

		void SendCurrentState(const function<Signature_>& slot) const
		{ _impl->SendCurrentState(function_storage(slot)); }

		TokenReturnProxy connect(const function<Signature_>& slot, bool sendCurrentState = true) const
		{
			TaskLifeToken token(_impl->CreateSyncToken());
			return _impl->Connect(function_storage(slot), token.GetExecutionTester(), token, sendCurrentState);
		}

		TokenReturnProxy connect(const ITaskExecutorPtr& worker, const function<Signature_>& slot, bool sendCurrentState = true) const
		{
			TaskLifeToken token(_impl->CreateAsyncToken());
			return _impl->Connect(function_storage(function<Signature_>(MakeAsyncFunction(worker, slot, token))), null, token, sendCurrentState);
		}
	};

	/** @} */

}

#endif
