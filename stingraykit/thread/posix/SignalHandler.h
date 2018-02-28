#ifndef STINGRAYKIT_THREAD_POSIX_SIGNALHANDLER_H
#define STINGRAYKIT_THREAD_POSIX_SIGNALHANDLER_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/Thread.h>
#include <stingraykit/SystemException.h>

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

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

#ifdef STINGRAY_SIGNAL_HANDLING_ENABLED

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

#else

	template < typename HandlerFuncHolder >
	class SignalHandlerSetter
	{
	private:
		int _signalNum;

	public:
		SignalHandlerSetter(int signalNum) : _signalNum(signalNum) { }

		int GetSignalNum() const { return _signalNum; }
	};

	inline bool SendSignal(const pthread_t& threadHandle, int signalNum) { return true; }

#endif

}}

#endif
