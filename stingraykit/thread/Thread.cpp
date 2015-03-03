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

	Thread::Thread(const std::string& name, const FuncType& threadFunc)
	{ _thread = ThreadEngine::BeginThread(threadFunc, name); }


	Thread::~Thread()
	{ _thread.reset(); }

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
	{ return ThreadEngine::GetCurrentThreadData()->GetThreadName(); }

	IThreadInfoPtr Thread::GetCurrentThreadInfo()
	{ return ThreadEngine::GetCurrentThreadInfo(); }

	Thread::ThreadStatsVec Thread::GetStats()
	{ return ThreadEngine::GetThreadsStats(); }

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
