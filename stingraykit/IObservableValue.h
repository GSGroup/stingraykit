#ifndef STINGRAYKIT_IOBSERVABLEVALUE_H
#define STINGRAYKIT_IOBSERVABLEVALUE_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/signal/signal_connector.h>

namespace stingray
{

	template < typename ValueType_ >
	struct IReadonlyObservableValue
	{
		typedef typename GetParamPassingType<ValueType_>::ValueT ParamPassingType;

		typedef void OnChangedSignature(ParamPassingType);

	public:
		virtual ~IReadonlyObservableValue() { }

		virtual ValueType_ Get() const = 0;

		virtual signal_connector<OnChangedSignature> OnChanged() const = 0;
		virtual const Mutex& GetSyncRoot() const = 0;
	};


	template < typename ValueType_ >
	struct IObservableValue : public virtual IReadonlyObservableValue<ValueType_>
	{
	private:
		typedef IReadonlyObservableValue<ValueType_> Base;

	public:
		typedef typename Base::ParamPassingType ParamPassingType;
		typedef typename Base::OnChangedSignature OnChangedSignature;

	public:
		virtual void Set(ParamPassingType value) = 0;
	};

}

#endif
