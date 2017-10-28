// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/diagnostics/AsyncProfiler.h>

#include <stingraykit/function/bind.h>
#include <stingraykit/time/TimeEngine.h>

namespace stingray
{

	AsyncProfiler::SessionImpl::SessionImpl(const char* name) :
		_name(name),
		_threadInfo(Thread::GetCurrentThreadInfo()),
		_startTime(TimeEngine::GetMonotonicMicroseconds())
	{ }

	AsyncProfiler::SessionImpl::SessionImpl(const optional<NameGetterFunc>& nameGetter) :
		_name(null),
		_nameGetter(nameGetter),
		_threadInfo(Thread::GetCurrentThreadInfo()),
		_startTime(TimeEngine::GetMonotonicMicroseconds())
	{ }



	STINGRAYKIT_DEFINE_NAMED_LOGGER(AsyncProfiler::Session);


	AsyncProfiler::Session::Session(const AsyncProfilerPtr& asyncProfiler, const char* name, size_t criticalMs) :
		_asyncProfiler(asyncProfiler),
		_nameHolder(name),
		_thresholdMs(criticalMs),
		_impl(_nameHolder.Get())
	{ _asyncProfiler->AddSession(_impl); }


	AsyncProfiler::Session::Session(const AsyncProfilerPtr& asyncProfiler, const std::string& name, size_t criticalMs) :
		_asyncProfiler(asyncProfiler),
		_nameHolder(name),
		_thresholdMs(criticalMs),
		_impl(_nameHolder.Get())
	{ _asyncProfiler->AddSession(_impl); }


	AsyncProfiler::Session::Session(const AsyncProfilerPtr& asyncProfiler, const NameGetterFunc& nameGetter, size_t criticalMs, const NameGetterTag&) :
		_asyncProfiler(asyncProfiler),
		_nameHolder(null),
		_thresholdMs(criticalMs),
		_impl(nameGetter)
	{ _asyncProfiler->AddSession(_impl); }


	AsyncProfiler::Session::~Session()
	{
		STINGRAYKIT_TRY("Can't remove session from profiler", _asyncProfiler->RemoveSession(_impl));

		try
		{
			TimeDuration duration(TimeDuration::FromMicroseconds(TimeEngine::GetMonotonicMicroseconds() - _impl.GetStartTime()));
			if (duration > TimeDuration::FromMilliseconds(_thresholdMs))
				s_logger.Warning() << _impl.GetName() << " took " << duration;
		}
		catch (const std::exception& ex)
		{ s_logger.Error() << ex; }
	}


	STINGRAYKIT_DEFINE_NAMED_LOGGER(AsyncProfiler);

	AsyncProfiler::AsyncProfiler(const std::string& threadName) :
		_timeoutMicroseconds(30 * 1000 * 1000),
		_thread(ThreadEngine::BeginThread(bind(&AsyncProfiler::ThreadFunc, this, _1), threadName))
	{ }


	AsyncProfiler::~AsyncProfiler()
	{ _thread.reset(); }


	void AsyncProfiler::ThreadFunc(const ICancellationToken& token)
	{
		MutexLock l(_mutex);
		while (token)
		{
			if (_sessions.empty())
			{
				_condition.Wait(_mutex, token);
				continue;
			}

			SessionImpl& top = *_sessions.begin();
			u64 timeNow = TimeEngine::GetMonotonicMicroseconds();
			if (timeNow < top.GetAbsoluteTimeout())
			{
				_condition.TimedWait(_mutex, TimeDuration::FromMicroseconds(top.GetAbsoluteTimeout() - timeNow));
				continue;
			}

			_sessions.erase(top);
			top.SetAbsoluteTimeout(TimeEngine::GetMonotonicMicroseconds() + _timeoutMicroseconds);
			_sessions.push_back(top);

			// copy all necessary data before releasing mutex
			std::string name(top.GetName());
			std::string tname(top.GetThreadName());
			u64 startTime = top.GetStartTime();
			std::string backtrace(top.GetBacktrace());

			MutexUnlock ul(l);
			s_logger.Error() << "Task " << name << " in thread " << tname << " is being executed for more than " << TimeDuration::FromMicroseconds(timeNow - startTime) << " invoked from: " << backtrace;
		}
	}


	void AsyncProfiler::AddSession(SessionImpl& session)
	{
		MutexLock l(_mutex);
		session.SetAbsoluteTimeout(TimeEngine::GetMonotonicMicroseconds() + _timeoutMicroseconds);
		_sessions.push_back(session);
		if (&*_sessions.begin() == &session)
			_condition.Broadcast();
	}


	void AsyncProfiler::RemoveSession(SessionImpl& session)
	{
		MutexLock l(_mutex);
		if (&*_sessions.begin() == &session)
			_condition.Broadcast();
		_sessions.erase(session);
	}

}
