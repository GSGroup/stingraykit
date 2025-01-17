#ifndef STINGRAYKIT_SIGNAL_SIGNAL_CONNECTOR_H
#define STINGRAYKIT_SIGNAL_SIGNAL_CONNECTOR_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/function/AsyncFunction.h>
#include <stingraykit/signal/signal_policies.h>
#include <stingraykit/Token.h>

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

			virtual Token Connect(const function_storage& func, const FutureExecutionTester& invokeTester, TaskLifeToken&& connectionToken, bool sendCurrentState) = 0;
			virtual void SendCurrentState(const function_storage& slot) const = 0;

			virtual TaskLifeToken CreateSyncToken() const = 0;
			virtual TaskLifeToken CreateAsyncToken() const = 0;
		};

	}


	template < typename Signature_, ConnectionPolicy::Enum ConnectionPolicy_ = ConnectionPolicy::Any >
	class signal_connector
	{
		STINGRAYKIT_DEFAULTCOPYABLE(signal_connector);
		STINGRAYKIT_DEFAULTMOVABLE(signal_connector);

		template < typename S, ConnectionPolicy::Enum P >
		friend class signal_connector;

	public:
		using Signature = Signature_;

		using RetType = void;
		using ParamTypes = typename function_info<Signature>::ParamTypes;

	private:
		self_count_ptr<Detail::ISignalConnector>	_impl;

	public:
		signal_connector() { }
		explicit signal_connector(const self_count_ptr<Detail::ISignalConnector>& impl) : _impl(STINGRAYKIT_REQUIRE_NOT_NULL(impl)) { }

		/// @brief Converts signal_connector<Signature_, ConnectionPolicy::Any> to sync_signal_connector or async_signal_connector
		template < ConnectionPolicy::Enum ConnectionPolicy__, typename EnableIf<ConnectionPolicy__ != ConnectionPolicy_ && ConnectionPolicy__ == ConnectionPolicy::Any, int>::ValueT = 0 >
		signal_connector(const signal_connector<Signature_, ConnectionPolicy__>& other) : _impl(other._impl) { }
		template < ConnectionPolicy::Enum ConnectionPolicy__, typename EnableIf<ConnectionPolicy__ != ConnectionPolicy_ && ConnectionPolicy__ == ConnectionPolicy::Any, int>::ValueT = 0 >
		signal_connector(signal_connector<Signature_, ConnectionPolicy__>&& other) : _impl(std::move(other._impl)) { }

		void SendCurrentState(const function<Signature_>& slot) const
		{
			if (STINGRAYKIT_LIKELY(_impl.is_initialized()))
				_impl->SendCurrentState(function_storage(slot));
		}

		template < ConnectionPolicy::Enum ConnectionPolicy__ = ConnectionPolicy_, typename EnableIf<ConnectionPolicy__ == ConnectionPolicy_ && (ConnectionPolicy__ == ConnectionPolicy::Any || ConnectionPolicy__ == ConnectionPolicy::SyncOnly), int>::ValueT = 0 >
		Token connect(const function<Signature_>& slot, bool sendCurrentState = true) const
		{
			if (STINGRAYKIT_UNLIKELY(!_impl))
				return Token();

			TaskLifeToken token(_impl->CreateSyncToken());
			const FutureExecutionTester tester(token.GetExecutionTester());

			return _impl->Connect(function_storage(slot), tester, std::move(token), sendCurrentState);
		}

		template < ConnectionPolicy::Enum ConnectionPolicy__ = ConnectionPolicy_, typename EnableIf<ConnectionPolicy__ == ConnectionPolicy_ && (ConnectionPolicy__ == ConnectionPolicy::Any || ConnectionPolicy__ == ConnectionPolicy::AsyncOnly), int>::ValueT = 0 >
		Token connect(const ITaskExecutorPtr& worker, const function<Signature_>& slot, bool sendCurrentState = true) const
		{
			if (STINGRAYKIT_UNLIKELY(!_impl))
				return Token();

			TaskLifeToken token(_impl->CreateAsyncToken());
			const FutureExecutionTester tester(token.GetExecutionTester());

			return _impl->Connect(function_storage(function<Signature_>(MakeAsyncFunction(worker, slot, tester))), null, std::move(token), sendCurrentState);
		}
	};


	template < typename Signature_ >
	using sync_signal_connector = signal_connector<Signature_, ConnectionPolicy::SyncOnly>;


	template < typename Signature_ >
	using async_signal_connector = signal_connector<Signature_, ConnectionPolicy::AsyncOnly>;


	namespace Detail
	{

		struct DummySignalConnectorProxy
		{
			template < typename Signature_, ConnectionPolicy::Enum ConnectionPolicy_ >
			operator signal_connector<Signature_, ConnectionPolicy_> () const
			{ return signal_connector<Signature_, ConnectionPolicy_>(); }
		};

	}


	inline Detail::DummySignalConnectorProxy make_dummy_signal_connector()
	{ return Detail::DummySignalConnectorProxy(); }

	/** @} */

}

#endif
