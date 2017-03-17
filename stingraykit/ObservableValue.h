#ifndef STINGRAYKIT_OBSERVABLEVALUE_H
#define STINGRAYKIT_OBSERVABLEVALUE_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/compare/comparers.h>
#include <stingraykit/core/NonCopyable.h>
#include <stingraykit/signal/signals.h>

namespace stingray
{

	template < typename T, typename EqualsCmp = comparers::Equals>
	class ObservableValue
	{
		STINGRAYKIT_NONASSIGNABLE(ObservableValue);

		typedef typename GetParamPassingType<T>::ValueT		ParamPassingType;

	private:
		T																					_val;
		EqualsCmp																			_equalsCmp;
		const shared_ptr<Mutex>																_mutex;
		signal<void(ParamPassingType), signal_policies::threading::ExternalMutexPointer>	_onChanged;

	public:
		explicit ObservableValue(ParamPassingType val = T()) :
			_val(val),
			_mutex(new Mutex()),
			_onChanged(signal_policies::threading::ExternalMutexPointer(_mutex), bind(&ObservableValue::OnChangedPopulator, this, _1))
		{ }

		ObservableValue& operator= (ParamPassingType val)
		{
			Set(val);
			return *this;
		}

		operator T() const
		{ return Get(); }

		void Set(ParamPassingType val)
		{
			signal_locker l(_onChanged);
			if (_equalsCmp(_val, val))
				return;
			_val = val;
			_onChanged(_val);
		}

		T Get() const
		{
			signal_locker l(_onChanged);
			return _val;
		}

		const Mutex& GetSyncRoot() const
		{ return *_mutex; }

		signal_connector<void(ParamPassingType)> OnChanged() const
		{ return _onChanged.connector(); }

	private:
		void OnChangedPopulator(const function<void(ParamPassingType)>& slot) const
		{ slot(_val); }
	};

}

#endif
