#ifndef STINGRAYKIT_SIGNAL_SIGNALTOFUTURE_H
#define STINGRAYKIT_SIGNAL_SIGNALTOFUTURE_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/signal/signal_connector.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

	template < typename ParamType >
	class SignalToFutureWrapper
	{
	private:
		promise<ParamType>				_promise;
		shared_future<ParamType>		_future;
		Token							_connection;

	public:
		template < typename SignalSignature >
		explicit SignalToFutureWrapper(const signal_connector<SignalSignature>& connector)
		{ _connection = connector.connect(WrapSetValue(_promise)); }

		shared_future<ParamType> GetFuture()
		{
			if (!_future.valid())
				_future = _promise.get_future().share();
			return _future;
		}

	private:
		template < typename ParamType__ >
		static auto WrapSetValue(promise<ParamType__>& promise_)
		{ return Bind(&SetValueProxy<ParamType__>, wrap_ref(promise_), _1); }

		static auto WrapSetValue(promise<void>& promise_)
		{ return Bind(&promise<void>::set_value, wrap_ref(promise_)); }

		template < typename ParamType__ >
		static void SetValueProxy(promise<ParamType__>& promise_, typename If<IsReference<ParamType__>::Value, ParamType__, const ParamType__&>::ValueT value)
		{ promise_.set_value(value); }
	};

	/** @} */

}

#endif
