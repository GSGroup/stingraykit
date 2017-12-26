// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/diagnostics/AsyncProfiler.h>

#include <stingraykit/function/bind.h>
#include <stingraykit/thread/TimedCancellationToken.h>
#include <stingraykit/time/TimeEngine.h>

namespace stingray
{

	namespace
	{

		TimeDuration GetMonotonic()
		{ return TimeDuration::FromMicroseconds(TimeEngine::GetMonotonicMicroseconds()); }

	}


	AsyncProfiler::SessionImpl::SessionImpl(const char* name)
		:	_name(name),
			_threadInfo(Thread::GetCurrentThreadInfo()),
			_startTime(GetMonotonic())
	{ }

	AsyncProfiler::SessionImpl::SessionImpl(const optional<NameGetterFunc>& nameGetter)
		:	_name(null),
			_nameGetter(nameGetter),
			_threadInfo(Thread::GetCurrentThreadInfo()),
			_startTime(GetMonotonic())
	{ }


	STINGRAYKIT_DEFINE_NAMED_LOGGER(AsyncProfiler::Session);

	AsyncProfiler::Session::Session(const AsyncProfilerPtr& asyncProfiler, const char* name, TimeDuration threshold)
		:	_asyncProfiler(asyncProfiler),
			_nameHolder(name),
			_threshold(threshold),
			_impl(_nameHolder.Get())
	{ _asyncProfiler->AddSession(_impl); }


	AsyncProfiler::Session::Session(const AsyncProfilerPtr& asyncProfiler, const std::string& name, TimeDuration threshold)
		:	_asyncProfiler(asyncProfiler),
			_nameHolder(name),
			_threshold(threshold),
			_impl(_nameHolder.Get())
	{ _asyncProfiler->AddSession(_impl); }


	AsyncProfiler::Session::Session(const AsyncProfilerPtr& asyncProfiler, const NameGetterFunc& nameGetter, TimeDuration threshold, const NameGetterTag&)
		:	_asyncProfiler(asyncProfiler),
			_nameHolder(null),
			_threshold(threshold),
			_impl(nameGetter)
	{ _asyncProfiler->AddSession(_impl); }


	AsyncProfiler::Session::~Session()
	{
		STINGRAYKIT_TRY("Can't remove session from profiler", _asyncProfiler->RemoveSession(_impl));

		try
		{
			const TimeDuration duration = GetMonotonic() - _impl.GetStartTime();
			if (duration > _threshold)
				s_logger.Warning() << _impl.GetName() << " took " << duration;
		}
		catch (const std::exception& ex)
		{ s_logger.Error() << "Can't report total execution time: " << ex; }
	}


	STINGRAYKIT_DEFINE_NAMED_LOGGER(AsyncProfiler);

	AsyncProfiler::AsyncProfiler(const std::string& threadName)
		:	_timeout(TimeDuration::FromSeconds(30)),
			_thread(ThreadEngine::BeginThread(bind(&AsyncProfiler::ThreadFunc, this, _1), threadName))
	{ }


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

			const TimeDuration timeNow = GetMonotonic();
			if (timeNow < top.GetAbsoluteTimeout())
			{
				_condition.Wait(_mutex, TimedCancellationToken(top.GetAbsoluteTimeout() - timeNow));
				continue;
			}

			_sessions.erase(top);
			top.SetAbsoluteTimeout(GetMonotonic() + _timeout);
			_sessions.push_back(top);

			// copy all necessary data before releasing mutex
			const std::string name(top.GetName());
			const std::string tname(top.GetThreadName());
			const TimeDuration startTime = top.GetStartTime();
			const std::string backtrace(top.GetBacktrace());

			MutexUnlock ul(l);
			s_logger.Error() << "Task " << name << " in thread " << tname << " is being executed for more than " << (timeNow - startTime) << " invoked from: " << backtrace;
		}
	}


	void AsyncProfiler::AddSession(SessionImpl& session)
	{
		MutexLock l(_mutex);
		session.SetAbsoluteTimeout(GetMonotonic() + _timeout);
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
