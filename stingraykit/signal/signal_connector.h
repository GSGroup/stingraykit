#ifndef STINGRAYKIT_SIGNAL_SIGNAL_CONNECTOR_H
#define STINGRAYKIT_SIGNAL_SIGNAL_CONNECTOR_H


#include <stingraykit/IToken.h>
#include <stingraykit/function/async_function.h>
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

		template < typename Signature_ >
		struct ISignalConnector : public self_counter<ISignalConnector<Signature_> >
		{
			virtual ~ISignalConnector() { }

			virtual Token Connect(const function<Signature_>& funcStorage, const FutureExecutionTester& futureExecutionTester, const TaskLifeToken& taskLifeToken, bool sendCurrentState) = 0;
			virtual void SendCurrentState(const function<Signature_>& slot) const = 0;
		};

	}

	template < typename Signature_ >
	class signal_connector
	{
	private:
		self_count_ptr<Detail::ISignalConnector<Signature_> >	_impl;

	public:
		signal_connector(const self_count_ptr<Detail::ISignalConnector<Signature_> >& impl)
			: _impl(impl)
		{ }

		void SendCurrentState(const function<Signature_>& slot) const
		{ _impl->SendCurrentState(slot); }

		Token connect(const function<Signature_>& slot, bool sendCurrentState = true) const
		{
			TaskLifeToken token;
			return _impl->Connect(slot, token.GetExecutionTester(), token, sendCurrentState);
		}

		Token connect(const ITaskExecutorPtr& worker, const function<Signature_>& slot, bool sendCurrentState = true) const
		{
			AsyncFunction<Signature_> slot_func(worker, slot);
			return _impl->Connect(slot_func, null, slot_func.GetToken(), sendCurrentState); // Using real execution tester instead of null may cause deadlocks!!!
		}
	};

	/** @} */

}

#endif
