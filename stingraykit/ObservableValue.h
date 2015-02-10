#ifndef STINGRAYKIT_OBSERVABLEVALUE_H
#define STINGRAYKIT_OBSERVABLEVALUE_H

#include <stingraykit/MetaProgramming.h>
#include <stingraykit/signal/signal_connector.h>
#include <stingraykit/signal/signals.h>
#include <stingraykit/toolkit.h>

namespace stingray
{


	template < typename T, typename EqualsCmp = comparers::Equals>
	class ObservableValue
	{
		STINGRAYKIT_NONASSIGNABLE(ObservableValue);

		typedef typename GetParamPassingType<T>::ValueT		ParamPassingType;

	private:
		T									_val;
		EqualsCmp							_equalsCmp;
		signal<void(ParamPassingType)>		_onChanged;

	public:
		ObservableValue()
			:	_val(),
				_onChanged(bind(&ObservableValue::OnChangedPopulator, this, _1))
		{ }

		ObservableValue(ParamPassingType val)
			:	_val(val),
				_onChanged(bind(&ObservableValue::OnChangedPopulator, this, _1))
		{ }

		ObservableValue& operator= (ParamPassingType val)
		{
			Set(val);
			return *this;
		}

		operator T() const
		{
			return Get();
		}

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

		signal_connector<void(ParamPassingType)> OnChanged() const { return _onChanged.connector(); }

	private:
		void OnChangedPopulator(const function<void(ParamPassingType)>& slot) const
		{ slot(_val); }
	};


}

#endif
