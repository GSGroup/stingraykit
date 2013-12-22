#ifndef STINGRAY_TOOLKIT_SIGNAL_PROXY_H
#define STINGRAY_TOOLKIT_SIGNAL_PROXY_H


#include <stingray/toolkit/signals.h>

namespace stingray
{

	template < typename Signature >
	class signal_proxy
	{
	public:
		typedef signal<Signature>	SignalType;
		typedef function<Signature>	FuncType;

	private:
		SignalType& _signal;

	public:
		signal_proxy(SignalType& s) : _signal(s)
		{}

		signal_connection connect(const ITaskExecutorPtr& executor, const FuncType& handler) const
		{ return _signal.connect(executor, handler); }

		signal_connection connect(const FuncType& handler) const
		{ return _signal.connect(handler); }
	};

}

#endif
