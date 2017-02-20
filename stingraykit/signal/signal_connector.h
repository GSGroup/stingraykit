#ifndef STINGRAYKIT_SIGNAL_SIGNAL_CONNECTOR_H
#define STINGRAYKIT_SIGNAL_SIGNAL_CONNECTOR_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/Token.h>
#include <stingraykit/function/AsyncFunction.h>
#include <stingraykit/signal/signal_policies.h>
#include <stingraykit/thread/ITaskExecutor.h>
#include <stingraykit/TaskLifeToken.h>


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

			virtual Token Connect(const function_storage& func, const FutureExecutionTester& invokeTester, const TaskLifeToken& connectionToken, bool sendCurrentState) = 0;
			virtual void SendCurrentState(const function_storage& slot) const = 0;

			virtual TaskLifeToken CreateSyncToken() const = 0;
			virtual TaskLifeToken CreateAsyncToken() const = 0;

			virtual ConnectionPolicy GetConnectionPolicy() const = 0;
		};

	}

	template < typename Signature_ >
	class signal_connector
	{
	public:
		typedef Signature_										Signature;

		typedef void											RetType;
		typedef typename function_info<Signature>::ParamTypes	ParamTypes;

	private:
		self_count_ptr<Detail::ISignalConnector>	_impl;

	public:
		signal_connector(const self_count_ptr<Detail::ISignalConnector>& impl) : _impl(impl)
		{ }

		void SendCurrentState(const function<Signature_>& slot) const
		{ _impl->SendCurrentState(function_storage(slot)); }

		Token connect(const function<Signature_>& slot, bool sendCurrentState = true) const
		{
			TaskLifeToken token(_impl->CreateSyncToken());
			STINGRAYKIT_CHECK(_impl->GetConnectionPolicy() == ConnectionPolicy::Any || _impl->GetConnectionPolicy() == ConnectionPolicy::SyncOnly, "sync-connect to async-only signal");
			return _impl->Connect(function_storage(slot), token.GetExecutionTester(), token, sendCurrentState);
		}

		Token connect(const ITaskExecutorPtr& worker, const function<Signature_>& slot, bool sendCurrentState = true) const
		{
			TaskLifeToken token(_impl->CreateAsyncToken());
			STINGRAYKIT_CHECK(_impl->GetConnectionPolicy() == ConnectionPolicy::Any || _impl->GetConnectionPolicy() == ConnectionPolicy::AsyncOnly, "async-connect to sync-only signal");
			return _impl->Connect(function_storage(function<Signature_>(MakeAsyncFunction(worker, slot, token.GetExecutionTester()))), null, token, sendCurrentState);
		}
	};

	/** @} */

}

#endif
