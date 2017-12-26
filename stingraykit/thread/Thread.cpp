// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/Thread.h>

#include <stingraykit/function/bind.h>
#include <stingraykit/diagnostics/AsyncProfiler.h>
#include <stingraykit/diagnostics/ExecutorsProfiler.h>


namespace stingray
{

	namespace
	{
		static unsigned ProfileReportThreshold = 3000;
	}

	EnableInterruptionPoints::EnableInterruptionPoints(bool value)
	{
		TLSData * tls = ThreadEngine::GetCurrentThreadData();
		_enabled = tls? tls->IsThreadCancellationEnabled(): false;
		if (_enabled)
			_prevValue = ThreadEngine::EnableInterruptionPoints(value);
	}

	EnableInterruptionPoints::~EnableInterruptionPoints()
	{
		if (_enabled)
			ThreadEngine::EnableInterruptionPoints(_prevValue);
	}

	////////////////////////////////////////

	Thread::Thread(const std::string& name, const FuncType& threadFunc, optional<TimeDuration> timeout)
	{ _thread = ThreadEngine::BeginThread(threadFunc, name, timeout); }


	Thread::~Thread()
	{
		AsyncProfiler::Session profile_session(ExecutorsProfiler::Instance().GetProfiler(), StringBuilder() % "Thread " % _thread->GetThreadInfo()->GetName() % " destructor", TimeDuration::FromSeconds(10));

		_thread.reset();
	}

	void Thread::Interrupt()
	{ _thread->Interrupt(); }

	IThread::ThreadId Thread::GetId()
	{ return _thread->GetId(); }

	void Thread::InterruptionPoint()
	{ ThreadEngine::InterruptionPoint(); }

	void Thread::Yield()
	{ ThreadEngine::Yield(); }

	void Thread::SleepMicroseconds(u64 microseconds)
	{ ThreadEngine::SleepMicroseconds(microseconds); }

	IThread::ThreadId Thread::GetCurrentThreadId()
	{ return ThreadEngine::GetCurrentThreadId(); }

	void Thread::SetCurrentThreadName(const std::string& name)
	{ ThreadEngine::SetCurrentThreadName(name); }

	const std::string& Thread::GetCurrentThreadName()
	{ return ThreadEngine::GetCurrentThreadName(); }

	IThreadInfoPtr Thread::GetCurrentThreadInfo()
	{ return ThreadEngine::GetCurrentThreadInfo(); }

	Thread::ThreadStatsVec Thread::GetStats()
	{ return ThreadEngine::GetThreadsStats(); }

	optional<SystemStats> Thread::GetSystemStats()
	{ return ThreadEngine::GetSystemStats(); }

	void Thread::SetCancellationToken(const ICancellationToken& token)
	{ ThreadEngine::GetCurrentThreadData()->SetCancellationToken(&token); }

	void Thread::ResetCancellationToken()
	{ ThreadEngine::GetCurrentThreadData()->SetCancellationToken(NULL); }

	const ICancellationToken& Thread::GetCancellationToken()
	{
		const ICancellationToken* token = ThreadEngine::GetCurrentThreadData()->GetCancellationToken();
		STINGRAYKIT_CHECK(token, InvalidOperationException());
		return *token;
	}


	Thread::PrioritySetter::PrioritySetter()
	{ }


	Thread::PrioritySetter::PrioritySetter(ThreadSchedulingParams params)
	{ Set(params); }


	Thread::PrioritySetter::~PrioritySetter()
	{ Reset(); }


	void Thread::PrioritySetter::Set(ThreadSchedulingParams params)
	{
		Reset();
		STINGRAYKIT_TRY("Can't set scheduling params!", _prevParams = ThreadEngine::SetCurrentThreadPriority(params));
	}


	void Thread::PrioritySetter::Reset()
	{
		if (_prevParams)
		{
			STINGRAYKIT_TRY("Can't restore scheduling params!", ThreadEngine::SetCurrentThreadPriority(*_prevParams));
			_prevParams.reset();
		}
	}

}
