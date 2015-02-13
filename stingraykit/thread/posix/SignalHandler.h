#ifndef STINGRAYKIT_THREAD_POSIX_SIGNALHANDLER_H
#define STINGRAYKIT_THREAD_POSIX_SIGNALHANDLER_H


#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#include <stingraykit/SystemException.h>

#ifdef __GNU_LIBRARY__
#	ifndef SIGCANCEL
#		define SIGCANCEL __SIGRTMIN
#	endif
#else
#	error please mask cancellation signal if thread cancellation was implemented via tgkill (which is true for linux/nptl)
#endif


namespace stingray {
namespace posix
{

	template < typename HandlerFuncHolder >
	class SignalHandlerSetter
	{
		int						_signalNum;
		struct sigaction		_myAct;
		struct sigaction		_oldAct;

	public:
		SignalHandlerSetter(int signalNum)
			: _signalNum(signalNum)
		{
			memset(&_myAct, 0, sizeof(_myAct));
			memset(&_oldAct, 0, sizeof(_oldAct));

			_myAct.sa_sigaction	= &HandlerFunc;
			_myAct.sa_flags		= SA_SIGINFO | SA_RESTART;
#ifdef __GNU_LIBRARY__
			sigemptyset(&_myAct.sa_mask);
			sigaddset(&_myAct.sa_mask, SIGCANCEL);
#endif
			STINGRAYKIT_CHECK(sigaction(_signalNum, &_myAct, &_oldAct) == 0, SystemException("sigaction"));
		}

		~SignalHandlerSetter()
		{
			if (sigaction(_signalNum, &_oldAct, NULL) != 0)
				perror(("sigaction(" + ToString(_signalNum) + ", ...): " + STINGRAYKIT_WHERE.ToString()).c_str());
		}

		int GetSignalNum() const
		{ return _signalNum; }

	private:
		static void HandlerFunc(int signalNum, siginfo_t* sigInfo, void* ctx)
		{
			TLSData::DisableThreadCancellationToken dtc(ThreadEngine::GetCurrentThreadData());
			HandlerFuncHolder::HandlerFunc(signalNum, sigInfo, ctx);
		}
	};

	inline bool SendSignal(const pthread_t& threadHandle, int signalNum)
	{ return (pthread_kill(threadHandle, signalNum) == 0); }


}}

#endif
